#ifndef EMB_RPC_SOCKET_SERVER_HPP
#define EMB_RPC_SOCKET_SERVER_HPP

#include <emb/net/socket.hpp>
#include <emb/net/async_server.hpp>
#include <emb/ser/serialize.hpp>
#include <atomic>

namespace emb::rpc
{

namespace detail
{

template<auto Id, class ProcTypes>
struct callback {
    static constexpr auto id = Id;
    typename ProcTypes::std_function callback;
    using arguments_tuple = typename ProcTypes::arguments_tuple;
    using return_type = typename ProcTypes::return_type;
};

}

template<std::size_t MaxClients, class Api>
class socket_server {
    template<auto Id>
    struct callback_set_proxy
    {
        callback_set_proxy(socket_server& srv)
            : srv_ {srv}
        {}

        template<class Func>
        void operator=(Func func) {
            srv_.set_callback<Id>(func);
        }

    private:
        socket_server& srv_;
    };
public:
    socket_server(emb::net::socket& sck)
        : multiplexer_ {sck, {*this, &socket_server::accepted, &socket_server::ready}}
        , running_{true}
    {}

    template<auto Id>
    auto callback() {
        return callback_set_proxy<Id>{*this};
    }

    template<auto Id, class Func>
    void set_callback(Func func) {
        set_callback_impl<Id>(func, std::make_index_sequence<std::tuple_size_v<callbacks_tuple>>{});
    }

    template<auto Id, class Func, std::size_t... I>
    void set_callback_impl(Func func, std::index_sequence<I...>) {
        auto set_if_id = [&](auto& cb) {
            if constexpr (cb.id == Id) {
                cb.callback = func;
            }
        };

        (set_if_id(std::get<I>(callbacks_)), ...);
    }

    void run()
    {
        while (running_) {
            run_once();
        }
    }

    void run_once()
    {
        multiplexer_.run_once();
    }

    void stop_running()
    {
        running_ = false;
    }

private:
    void accepted(emb::net::socket&) {
    }

    void ready(int, emb::net::socket& sck) {
        auto id = sck.read<int>();
        auto size = sck.read<std::size_t>();
        sck.read({buffer_.data(), buffer_.size()}, size);
        receive(id, sck, std::make_index_sequence<std::tuple_size_v<callbacks_tuple>>{});
    }

    template<std::size_t C, std::size_t... Idx>
    void receive(int id, emb::net::socket& sck, std::index_sequence<C, Idx...>) {
        if (static_cast<decltype(std::get<C>(callbacks_).id)>(id) == std::get<C>(callbacks_).id) {
            using callback_type = typename std::tuple_element_t<C, callbacks_tuple>;
            typename callback_type::arguments_tuple tup;
            ser::deserialize(tup, buffer_.data(), buffer_.data() + buffer_.size());

            if constexpr (std::is_same_v<typename callback_type::return_type, void>) {
                std::apply(std::get<C>(callbacks_).callback, tup);
            } else {
                auto result = std::apply(std::get<C>(callbacks_).callback, tup);
                std::tuple<typename callback_type::return_type> resp{result};
                auto size = ser::serialize(resp, buffer_.data(), buffer_.data() + buffer_.size());
                sck.write(size);
                sck.write(buffer_.data(), size);
            }
            return;
        }

        receive(id, sck, std::index_sequence<Idx...>{});
    }

    void receive(int, emb::net::socket&, std::index_sequence<>){}

    struct transform {
        template<class T>
        struct type_impl {
            using procedure_types = decltype(T::get_types());
            using result = detail::callback<
                T::get_id(),
                procedure_types>;
        };

        template<class T>
        using type = typename type_impl<T>::result;
    };

    using callbacks_tuple = typename decltype(Api{}.template transform<transform>())::as_tuple;

private:
    emb::net::async_server<
        emb::net::method_dispatcher<socket_server<MaxClients, Api>>,
        emb::net::static_sockets_buffer<MaxClients>>
            multiplexer_;
    callbacks_tuple callbacks_;
    std::array<unsigned char, 512> buffer_;
    std::atomic<bool> running_;
};

template<std::size_t N, class Api>
auto make_socket_server(Api, emb::net::socket& sck) {
    return socket_server<N, Api>(sck);
}

}

#endif // EMB_RPC_SOCKET_SERVER_HPP