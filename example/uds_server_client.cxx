#include "emb/net/uds_server_socket.hpp"
#include "emb/net/uds_client_socket.hpp"
#include "emb/net/async_server.hpp"
#include <thread>
#include <iostream>
#include <unistd.h>

int main()
{
    std::thread server_thread {
        []() {
            emb::net::uds_server_socket sck{"/tmp/test.sock"};
            emb::net::async_server<
                emb::net::function_dispatcher,
                emb::net::static_sockets_buffer<8>> mux {
                sck,
                {
                    [](auto& sck) {
                        std::cout << "Accepted" << std::endl;
                    },
                    [](auto id, emb::net::socket& sck) {
                        auto d = sck.read<32>();
                        std::cout << d.data() << std::endl;
                    }
                }
            };

            mux.run_once();
            mux.run_once();
        }
    };

    sleep(1);
    emb::net::uds_client_socket cli{"/tmp/test.sock"};
    cli.write("test");

    server_thread.join();
}
