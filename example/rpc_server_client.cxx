#include <emb/rpc/socket_server.hpp>
#include <emb/rpc/socket_client.hpp>
#include <emb/rpc/api.hpp>
#include <emb/net/uds_server_socket.hpp>
#include <emb/net/uds_client_socket.hpp>
#include <thread>
#include <iostream>

// a simple API definition, wrapped in structure for convenience
struct SimpleApi
{
    // exposed functions
    enum class Function {
        add,
        mul,
        print,
        stop
    };

    // create API object for use in emb::rpc subsystem
    static constexpr auto make() {
        using namespace emb::rpc;
        return make_api(
            // map function ID to function signature
            procedure<Function::add,            int  (int, int)>{},
            procedure<Function::mul,            int  (int, int)>{},
            procedure<Function::print,          void (std::string)>{},
            procedure<Function::stop,           void ()>{}
        );
    }
};

class Server
{
public:
    explicit Server(const char* socket_path)
        : socket_ {socket_path}
        , rpc_ {socket_}
    {
        rpc_.callback<SimpleApi::Function::add>() = [this](int a, int b) { return add(a, b); };
        rpc_.callback<SimpleApi::Function::mul>() = [this](int a, int b) { return mul(a, b); };
        rpc_.callback<SimpleApi::Function::print>() = [this]( auto const& str) { return print(str); };
        rpc_.callback<SimpleApi::Function::stop>() = [this] { stop(); };
    }

    void run() {
        thread_ = std::thread { [this] { rpc_.run(); } };
    }

    void join() {
        thread_.join();
    }

private:
    int add(int a, int b) {
        return a + b;
    }

    int mul(int a, int b) {
        return a * b;
    }

    void print(std::string const& str) {
        std::cout << "got string: " << str << std::endl;
    }

    void stop() {
        rpc_.stop_running();
    }

private:
    emb::net::uds_server_socket socket_;
    emb::rpc::socket_server<decltype(SimpleApi::make())> rpc_;
    std::thread thread_;
};

int main()
{
    using namespace std::string_literals;

    const char* socket_name = "/tmp/test.sock";
    Server srv{socket_name};

    // create and connect RPC client
    emb::net::uds_client_socket cs{socket_name};
    auto cli = emb::rpc::make_socket_client(SimpleApi::make(), cs);

    // run the server in its own thread
    srv.run();

    // remotely call the server API
    std::cout << "Remote add result (40 + 2) = " << cli.call<SimpleApi::Function::add>(40, 2) << std::endl;
    std::cout << "Remote mul result (21 * 2) = " << cli.call<SimpleApi::Function::mul>(21, 2) << std::endl;
    cli.call<SimpleApi::Function::print>("Hello server!"s);

    cli.call<SimpleApi::Function::stop>();
    srv.join();
}