#ifndef EMB_NET_ASYNC_SERVER_HPP
#define EMB_NET_ASYNC_SERVER_HPP

#include <emb/net/socket.hpp>
#include <functional>
#include <stdexcept>
#include <tuple>
#include <sys/epoll.h>

namespace emb::net
{
class function_dispatcher
{
public:
    using accept_function = std::function<void (socket&)>;
    using ready_function = std::function<void (int, socket&)>;

    function_dispatcher(accept_function accept, ready_function ready)
        : accept_ {accept}
        , ready_ {ready}
    {}

    void accepted(socket& sock) {
        accept_ (sock);
    }

    void ready(int id, socket& sock) {
        ready_ (id, sock);
    }
private:
    accept_function     accept_;
    ready_function      ready_;
};

template<class C>
class method_dispatcher
{
public:
    using accept_function = void (C::*)(socket&);
    using ready_function = void (C::*)(int, socket&);

    method_dispatcher(C& handler, accept_function accept, ready_function ready)
        : handler_ {handler}
        , accept_ {accept}
        , ready_ {ready}
    {}

    void accepted(socket& sock) {
        (handler_.*accept_) (sock);
    }

    void ready(int id, socket& sock) {
        (handler_.*ready_) (id, sock);
    }

private:
    C& handler_;
    accept_function accept_;
    ready_function ready_;
};

class async_server_base
{
public:
    using client_id = int;

    explicit async_server_base(socket& server_socket);

    void run();
    void run_once();

protected:
    void accept_new_client();

    virtual void call_accepted(socket& s) = 0;
    virtual void call_ready(client_id id, socket& s) = 0;

    virtual client_id add_client(socket&& sck) = 0;
    virtual bool has_space() const = 0;
    virtual socket& get_socket(client_id id) = 0;

private:
    static constexpr auto MAX_EVENTS = 32;
    socket&             server_socket_;
    int                 epoll_socket_;
    epoll_event         events_[MAX_EVENTS];
};

template<class Dispatcher, class SocketStorage>
class async_server final : public async_server_base
{
public:
    async_server(socket& server_socket, Dispatcher&& dispatcher)
        : async_server_base {server_socket}
        , dispatcher_ {std::move(dispatcher)}
    {
    }

protected:
    void call_accepted(socket& s) override
    {
        dispatcher_.accepted(s);
    }

    void call_ready(client_id id, socket& s) override
    {
        dispatcher_.ready(id, s);
    }

    client_id add_client(socket&& sck)
    {
        return storage_.add_client(std::move(sck));
    }

    bool has_space() const
    {
        return storage_.has_space();
    }

    socket& get_socket(client_id id)
    {
        return storage_.get_socket(id);
    }

private:
    Dispatcher          dispatcher_;
    SocketStorage       storage_;
};

template<std::size_t N>
class static_sockets_buffer {
public:
    static_assert(N > 0);

    async_server_base::client_id add_client(socket&& sck)
    {
        sockets_ [count_] = std::move(sck);
        return count_++;
    }

    bool has_space() const
    {
        return count_ < N;
    }

    socket& get_socket(async_server_base::client_id id)
    {
        return sockets_[id];
    }

private:
    std::array<socket, N>   sockets_;
    std::size_t             count_ = 0;
};

}

#endif // EMB_NET_ASYNC_SERVER_HPP