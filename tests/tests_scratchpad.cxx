#include "catch.hpp"
#include "emb/net/async_socket_server.hpp"
#include "emb/net/uds_client_socket.hpp"
#include "emb/net/uds_server_socket.hpp"
#include <thread>
#include <iostream>

TEST_CASE("a test", "[component][scratch]")
{
    constexpr auto socket_path = "/tmp/test.socket";
    emb::net::uds_server_socket srv_socket{socket_path};
    emb::net::async_socket_server async{srv_socket};

    std::thread th {
        [&] {
            std::array<unsigned char, 5> arr{'T', 'e', 's', 't', 0};
            async.on_accept = [&](emb::net::async_core::node_id id) {
                async.write(
                    id,
                    emb::contiguous_buffer(arr),
                    [] {}
                );
            };

            async.run_once();
            async.run_once();
        }
    };

    emb::net::uds_client_socket cli{socket_path};
    auto [size, data] = cli.read_some<64>();

    std::cout << data.data() << std::endl;

    th.join();
}