#include "emb/net/async_core.hpp"
#include <unistd.h>

namespace emb::net
{

async_core::async_core()
    : epoll_socket_ {epoll_create1(0)}
{
    if (epoll_socket_ == -1) {
        throw std::runtime_error{"epoll_create1() failed"};
    }
}

async_core::~async_core()
{
    ::close(epoll_socket_);
}

namespace
{
auto async_watch_to_epoll(async_watch v)
{
    switch (v)
    {
    case async_watch::err: return EPOLLERR;
    case async_watch::hup: return EPOLLHUP;
    case async_watch::in: return EPOLLIN;
    case async_watch::out: return EPOLLOUT;
    }

    return static_cast<EPOLL_EVENTS>(0);
}

std::uint32_t async_watch_list_to_mask(async_watch_flags watch_for)
{
    std::uint32_t mask = 0;
    auto or_value = [&](async_watch f, std::uint32_t val) {
        if (watch_for.is(f))
            mask |= val;
    };

    or_value(async_watch::err, EPOLLERR);
    or_value(async_watch::hup, EPOLLHUP);
    or_value(async_watch::in, EPOLLIN);
    or_value(async_watch::out, EPOLLOUT);

    return mask;
}

auto event_to_async_watch(std::uint32_t bitmask)
{
    async_watch_flags result;
    if (bitmask & EPOLLERR) result.set<async_watch::err>();
    if (bitmask & EPOLLHUP) result.set<async_watch::hup>();
    if (bitmask & EPOLLIN) result.set<async_watch::in>();
    if (bitmask & EPOLLOUT) result.set<async_watch::out>();

    return result;
}
}

void async_core::add(std::uint32_t id, net::socket& s, async_watch_flags watch_for)
{
    epoll_event ev;
    ev.events = async_watch_list_to_mask(watch_for);
    ev.data.u32 = id;

    if (epoll_ctl(epoll_socket_, EPOLL_CTL_ADD, s.descriptor(), &ev) == -1) {
        throw std::runtime_error("epoll_ctl(EPOLL_CTL_ADD) failed");
    }
}

void async_core::modify(std::uint32_t id, net::socket& s, async_watch_flags watch_for)
{
    epoll_event ev;
    ev.events = async_watch_list_to_mask(watch_for);
    ev.data.u32 = id;

    if (epoll_ctl(epoll_socket_, EPOLL_CTL_MOD, s.descriptor(), &ev) == -1) {
        throw std::runtime_error("epoll_ctl(EPOLL_CTL_ADD) failed");
    } 
}

void async_core::remove(net::socket& s)
{
    if (epoll_ctl(epoll_socket_, EPOLL_CTL_DEL, s.descriptor(), nullptr) == -1) {
        throw std::runtime_error("epoll_ctl(EPOLL_CTL_DEL) failed");
    }
}

void async_core::run_once()
{
    int number = epoll_wait(epoll_socket_, events_, MAX_EVENTS, -1);

    if (!callback_.is_set()) {
        return;
    }

    for (int i = 0; i < number; ++i) {
        auto const& d = events_[i].data;
        callback_.call(d.u32, event_to_async_watch(events_[i].events));
    }
}


}