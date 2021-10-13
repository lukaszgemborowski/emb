#include "emb/net/uds_server_socket.hpp"
#include "emb/net/uds_client_socket.hpp"
#include "emb/net/async_server.hpp"
#include <thread>
#include <iostream>

int main()
{
    std::thread server_thread {
        []() {
            emb::uds_server_socket sck{"/tmp/test.sock"};
            emb::async_server<emb::function_dispatcher> mux {
                sck,
                {
                    [](auto& sck) {
                        std::cout << "Accepted" << std::endl;
                    },
                    [](auto id, auto& sck) {
                        auto d = sck.read(32);
                        std::cout << d.data() << std::endl;
                    }
                }
            };

            mux.run_once();
            mux.run_once();
            /*emb::io_multiplexer mux;
            std::vector<emb::socket> clients;

            mux.add(sck, [&]() {
                std::cout << "IOMUX notified" << std::endl;
                clients.push_back(sck.accept());
                mux.add(clients.back(), [&socket = clients.back()](){
                    auto d = socket.read(32);
                    std::cout << d.data() << std::endl;
                });
            });

            mux.run_once();
            mux.run_once();*/
        }
    };

    sleep(1);
    emb::uds_client_socket cli{"/tmp/test.sock"};
    cli.write("test");

    server_thread.join();
}
