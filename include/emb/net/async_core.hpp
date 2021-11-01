#ifndef EMB_NET_ASYNC_CORE_HPP
#define EMB_NET_ASYNC_CORE_HPP

#include <emb/net/socket.hpp>
#include <emb/callback.hpp>
#include <emb/flags.hpp>
#include <functional>
#include <vector>
#include <sys/epoll.h>

namespace emb::net
{

enum class async_watch : std::uint32_t {
    invalid = 0,
    in  = 1 << 0,
    out = 1 << 1,
    err = 1 << 2,
    hup = 1 << 3
};

using async_watch_flags = emb::flags<async_watch>;

class async_core
{
private:
    emb::callback<void (std::uint32_t, async_watch_flags)> callback_;

public:
    using node_id = std::uint32_t;

    async_core();
    ~async_core();

    void run_once();
    void add(std::uint32_t id, net::socket& s, async_watch_flags watch_for);
    void modify(std::uint32_t id, net::socket& s, async_watch_flags watch_for);
    void remove(net::socket& s);

    emb::public_callback<void (std::uint32_t, async_watch_flags)> on_event {callback_};

private:
    static constexpr auto MAX_EVENTS = 32;
    int                     epoll_socket_;
    epoll_event             events_[MAX_EVENTS];
};

}

#endif // EMB_NET_ASYNC_CORE_HPP