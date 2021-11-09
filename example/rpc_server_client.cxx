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

    // a shortcut, with this small util we don't need to write emb::rpc::procedure everytime,
    // proc<> wil be enough in this "scope"
    template<auto Id, class T>
    using proc = emb::rpc::procedure<Id, T>;

    // define an API
    using definition = emb::rpc::api<
        // this is mapping from ID to a function prototype, eg Function::add is function accepting
        // two int's as argument and returning single int
        proc<Function::add,            int  (int, int)>,
        proc<Function::mul,            int  (int, int)>,
        proc<Function::print,          void (std::string)>,
        proc<Function::stop,           void ()>
    >;
};

// Example RPC server implementation
class Server
{
public:
    explicit Server(const char* socket_path)
        : socket_ {socket_path}
        , rpc_ {socket_}
    {
        // register callbacks for API functions, lambda functions must match defined API,
        // therefore we're just delegating calls to member functions ...
        rpc_.callback<SimpleApi::Function::add>() = [this](int a, int b) { return add(a, b); };
        rpc_.callback<SimpleApi::Function::mul>() = [this](int a, int b) { return mul(a, b); };
        rpc_.callback<SimpleApi::Function::print>() = [this]( auto const& str) { return print(str); };

        // ... or write a callback completly as lambda
        rpc_.callback<SimpleApi::Function::stop>() = [this] {
            std::cout << "Stop requested." << std::endl;
            rpc_.stop_running();
        };
    }

    void run() {
        // run a server in separate thread
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

private:
    emb::net::uds_server_socket socket_;
    emb::rpc::socket_server<SimpleApi::definition> rpc_;
    std::thread thread_;
};

int main()
{
    using namespace std::string_literals;

    const char* socket_name = "/tmp/test.sock";

    // create a server
    Server srv{socket_name};

    // create and connect RPC client
    emb::net::uds_client_socket cs{socket_name};
    emb::rpc::socket_client<SimpleApi::definition> cli{cs};

    // run the server in its own thread
    srv.run();

    // remotely call the server API, the call<ID>() will block until the result is received
    // therefore we can use them almost the same way as "local" functions
    std::cout << "Remote add result (40 + 2) = " << cli.call<SimpleApi::Function::add>(40, 2) << std::endl;
    std::cout << "Remote mul result (21 * 2) = " << cli.call<SimpleApi::Function::mul>(21, 2) << std::endl;
    cli.call<SimpleApi::Function::print>("Hello server!"s);

    cli.call<SimpleApi::Function::stop>();

    // wait for theserver thread to join
    srv.join();
}