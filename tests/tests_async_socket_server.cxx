#include "emb/net/async_socket_server.hpp"
#include "emb/net/uds_client_socket.hpp"
#include "emb/net/uds_server_socket.hpp"
#include "emb/ser/serialize.hpp"
#include <thread>
#include "catch.hpp"

namespace
{
using request = std::tuple<int, int>;
using response = std::tuple<int>;

class server
{
public:
    explicit server(const char* path)
        : socket_ {path}
        , async_ {socket_}
    {
        async_.on_accept = [this](auto id) { accepted(id); };
    }

    void run_once() {
        async_.run_once();
    }

    auto clients_count() const {
        return async_.clients_count();
    }

private:
    void read_next(emb::net::async_core::node_id id)
    {
        auto slot_id = slot(id);
        if (slot_id == -1) {
            return;
        }

        auto& e = clients_[slot_id];
        async_.read(
            id,
            emb::contiguous_buffer{e.in_buffer_, emb::ser::size_requirements(request{}).min},
            [id, slot_id, this]() {
                auto& e = clients_[slot_id];
                request req;
                emb::ser::deserialize(req, e.in_buffer_);

                response resp;
                std::get<0>(resp) = std::get<0>(req) + std::get<1>(req);
                auto resp_size = emb::ser::serialize(resp, e.out_buffer_);

                async_.write(
                    id,
                    emb::contiguous_buffer{e.out_buffer_, resp_size},
                    [] {}
                );
            }
        );
    }

    void accepted(emb::net::async_core::node_id id)
    {
        auto slot = empty_slot();
        if (slot == -1) {
            return;
        }

        auto &e = clients_[slot];
        e.id = id;
        read_next(id);
    }

    int empty_slot() const {
        for (int i = 0; i < clients_.size(); ++i) {
            if (clients_[i].id == 0) {
                return i;
            }
        }

        return -1;
    }

    int slot(int id) const {
        for (int i = 0; i < clients_.size(); ++i) {
            if (clients_[i].id == id) {
                return i;
            }
        }

        return -1;
    }

private:
    struct client {
        emb::net::async_core::node_id id = 0;
        std::array<unsigned char, 64> out_buffer_;
        std::array<unsigned char, 64> in_buffer_;
    };

    emb::net::uds_server_socket socket_;
    emb::net::async_socket_server async_;
    std::array<client, 32> clients_;
};
}

TEST_CASE("async socket server", "[async_socket_server][component]")
{
    constexpr auto path = "/tmp/test.socket";
    server srv{path};

    std::thread thr {
        [&] {
            for (int i = 0; i < 4; i ++) {
                srv.run_once();
            }
        }
    };

    std::array<unsigned char, 64> buff;
    emb::net::uds_client_socket cli{path};

    // send request
    request req{40, 2};
    auto size = emb::ser::serialize(req, buff);
    cli.write_some(emb::contiguous_buffer{buff, size});

    // read response
    response resp;
    cli.read(emb::contiguous_buffer{buff, emb::ser::size_requirements(resp).min});
    emb::ser::deserialize(resp, buff);

    CHECK(std::get<0>(resp) == 42);
    cli.close();

    thr.join();
    CHECK(srv.clients_count() == 0);
}