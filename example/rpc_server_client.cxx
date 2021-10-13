#include <emb/rpc/socket_server.hpp>
#include <emb/rpc/socket_client.hpp>
#include <emb/rpc/api.hpp>
#include <emb/net/uds_server_socket.hpp>
#include <emb/net/uds_client_socket.hpp>
#include <thread>
#include <iostream>

// api definition
enum class ApiFunction {
    add, print, stop
};

constexpr auto api = []{
    using namespace emb::rpc;
    return make_api(
        procedure<ApiFunction::add,     int  (int, int)>{},
        procedure<ApiFunction::print,   void (char)>{},
        procedure<ApiFunction::stop,    void()>{}
    );
}();

int main()
{
    // create server socket and RPC server object
    emb::net::uds_server_socket sck{"/tmp/test.sock"};
    auto rpc = emb::rpc::make_socket_server<4>(api, sck);

    // define RPC callbacks (API hooks)
    rpc.callback<ApiFunction::print>() = [](char c) {
        std::cout << "ApiFunction::print received: '" << c << "'" << std::endl;
    };

    rpc.callback<ApiFunction::add>() = [](int a, int b) { 
        std::cout << "ApiFunction::add received: " << a << " + " << b << std::endl;
        return a + b;
    };

    rpc.callback<ApiFunction::stop>() = [&]() { 
        std::cout << "stop requested" << std::endl;
        rpc.stop_running();
    };

    // start server in a separate thread
    std::thread server_thread {[&]{ rpc.run(); }};

    // create and connect RPC client
    emb::net::uds_client_socket cs{"/tmp/test.sock"};
    auto cli = emb::rpc::make_socket_client(api, cs);

    // remotely call the server API
    cli.call<ApiFunction::print>('x');
    auto r = cli.call<ApiFunction::add>(40, 2);
    std::cout << "result " << r << std::endl;
    cli.call<ApiFunction::stop>();

    server_thread.join();
}