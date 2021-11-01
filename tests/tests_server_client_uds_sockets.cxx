#include "emb/net/uds_server_socket.hpp"
#include "emb/net/uds_client_socket.hpp"
#include <thread>
#include <vector>
#include "catch.hpp"

TEST_CASE("Connection, transfer, disconnection", "[component][socket][uds_server_socket][uds_client_socket]")
{
    constexpr auto socket_name = "/tmp/emb-test.socket";
    const char data[] = "data to transfer";
    char dest1[sizeof(data)];
    char dest2[sizeof(data)];
    emb::net::uds_server_socket srv{socket_name};

    std::thread srv_thread {
        [&] {
            auto c = srv.accept();
            CHECK(c.is_null() == false);
            c.read(emb::contiguous_buffer{dest1, sizeof(dest1)});
            CHECK(std::memcmp(data, dest1, sizeof(data)) == 0);

            c.write_some(emb::contiguous_buffer{data, sizeof(data)});

            // read data again, expect closed socket
            REQUIRE_THROWS_AS(
                c.read(emb::contiguous_buffer{dest1, sizeof(dest1)}),
                emb::net::connection_closed
            );
        }
    };

    emb::net::uds_client_socket cli{socket_name};
    cli.write_some(emb::contiguous_buffer{data, sizeof(data)});
    cli.read(emb::contiguous_buffer{dest2, sizeof(dest2)});
    CHECK(std::memcmp(data, dest2, sizeof(data)) == 0);
    cli.close();

    srv_thread.join();
}