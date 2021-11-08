#ifndef EMB_RPC_SOCKET_SERVER_HPP
#define EMB_RPC_SOCKET_SERVER_HPP

#include <emb/net/socket.hpp>
#include <emb/net/async_socket_server.hpp>
#include <emb/ser/serialize.hpp>
#include <emb/rpc/callback_tuple.hpp>
#include <emb/rpc/protocol.hpp>
#include <emb/cpp/tuple.hpp>
#include <atomic>


#include <iostream>

namespace emb::rpc
{

template<class Api>
class socket_server {
    template<auto Id>
    struct callback_set_proxy {
        callback_set_proxy(callback_tuple<Api>& ct)
            : ct_ {ct}
        {}

        template<class Fun>
        void operator=(Fun fun) {
            ct_.template set_callback<Id>(fun);
        }
    private:
        callback_tuple<Api>& ct_;
    };

    struct client_buffers {
        net::async_core::node_id id = 0;
        std::array<unsigned char, 256> in_buff;
        std::array<unsigned char, 256> out_buff;
    };
public:
    socket_server(emb::net::socket& sck)
        : server_ {sck}
        , running_ {true}
    {
        server_.on_accept = [this](auto id) { accept(id); };
    }

    void run() {
        while (running_) {
            run_once();
        }
    }

    void run_once() {
        server_.run_once();
    }

    void stop_running() {
        running_ = false;
    }

    template<auto Id>
    auto callback() {
        return callback_set_proxy<Id>{callbacks_};
    }

private:
    void accept(net::async_core::node_id id) {
        auto slot_id = find_empty_slot();
        if (slot_id == -1) {
            return;
        }

        auto& slot = clients_[slot_id];
        slot.id = id;
        shedule_next_request(slot);
    }

    void shedule_next_request(client_buffers &slot) {
        server_.read(
            slot.id,
            emb::contiguous_buffer{slot.in_buff, sizeof(ser::tuple_size_type)},
            [&slot, this] { handle_request_header(slot); }
        );
    }

    void handle_request_header(client_buffers &slot) {
        auto payload_size = ser::deserialize_size(emb::contiguous_buffer{slot.in_buff});

        std::cout << "got header, size = " << payload_size << std::endl;

        server_.read(
            slot.id,
            emb::contiguous_buffer{slot.in_buff}.slice(sizeof(ser::tuple_size_type), payload_size - sizeof(ser::tuple_size_type)),
            [&slot, this] { handle_request_data(slot); }
        );
    }

    void handle_request_data(client_buffers &slot) {
        std::tuple<int> call_id {-1};
        ser::deserialize(call_id, slot.in_buff); // TODO: check

        std::cout << "got msg id " << std::get<0>(call_id) << std::endl;

        callbacks_.visit(
            std::get<0>(call_id),
            [&slot, this](auto& callback_data) {
                using callback_type = std::decay_t<decltype(callback_data)>;

                // message in form of [id, args...]
                auto message = std::tuple_cat(std::tuple<int>{},  typename callback_type::arguments_tuple{});
                ser::deserialize(message, slot.in_buff);

                // transform [id, args..] to [args...]
                auto arguments = cpp::tuple_slice<1, std::tuple_size_v<decltype(message)> - 1>(message);

                if constexpr (std::is_same_v<void, typename callback_type::return_type>) {
                    std::apply(callback_data.callback, arguments);
                    shedule_next_request(slot);
                } else {
                    std::tuple<typename callback_type::return_type> response{
                        std::apply(callback_data.callback, arguments)
                    };

                    auto size = ser::serialize(response, emb::contiguous_buffer{slot.out_buff}.slice(sizeof(std::size_t), emb::npos));
                    std::memcpy(slot.out_buff.data(), &size, sizeof(size));

                    server_.write(
                        slot.id,
                        emb::contiguous_buffer{slot.out_buff, size + sizeof(size)},
                        [&slot, this] {
                            shedule_next_request(slot);
                        }
                    );
                }
            }
        );
    }

    std::size_t find_empty_slot() {
        for (std::size_t i = 0; i < MAX_CLIENTS; ++i) {
            if (clients_[i].id == 0) {
                return i;
            }
        }

        return -1;
    }

private:
    emb::net::async_socket_server   server_;
    callback_tuple<Api>             callbacks_;
    std::atomic<bool>               running_;
    static constexpr auto MAX_CLIENTS = 32;
    std::array<client_buffers, MAX_CLIENTS> clients_;
};

template<class Api>
auto make_socket_server(Api, emb::net::socket& sck) {
    return socket_server<Api>(sck);
}

}

#endif // EMB_RPC_SOCKET_SERVER_HPP