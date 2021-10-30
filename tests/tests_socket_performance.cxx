#include "catch.hpp"
#include "emb/net/uds_server_socket.hpp"
#include "emb/net/uds_client_socket.hpp"
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>

namespace {
template<std::size_t Chunk, std::size_t Count>
void client_server_test()
{
    constexpr auto socket_name = "/tmp/emb-test.socket";
    emb::net::uds_server_socket srv{socket_name};

    std::thread srv_thread {
        [&srv] {
            auto c = srv.accept();
            std::vector<unsigned char> buff;
            buff.resize(Chunk);

            for (int i = 0; i < Count; ++i) {
                c.write_some(emb::contiguous_buffer{buff});
            }
        }
    };

    emb::net::uds_client_socket cli{socket_name};
    std::vector<unsigned char> rcv_buffer;
    rcv_buffer.resize(Chunk);

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < Count; ++i) {
        cli.read(emb::contiguous_buffer{rcv_buffer});
    }

    srv_thread.join();

    auto time_diff = std::chrono::high_resolution_clock::now() - start;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_diff).count();

    std::cout << "Client-server throughtput, chunk-size=" << Chunk << ", count=" << Count << ", time=" <<
        ms << "ms, time per chunk=" << (ms/Count) << "ms" << std::endl;
}
}

TEST_CASE("Client-server throughtput", "[component][performance][socket][uds_server_socket][uds_client_socket]")
{
    client_server_test<(1 << 8 ), 10000>();
    client_server_test<(1 << 9 ), 10000>();
    client_server_test<(1 << 10), 10000>();
    client_server_test<(1 << 11), 10000>();
    client_server_test<(1 << 12), 10000>();
    client_server_test<(1 << 13), 10000>();
    client_server_test<(1 << 14), 10000>();
}
