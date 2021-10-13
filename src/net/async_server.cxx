#include "emb/net/async_server.hpp"

namespace emb::net
{
async_server_base::async_server_base(socket& server_socket)
    : server_socket_ {server_socket}
    , epoll_socket_ {epoll_create1(0)}
    , events_ {}
{
    if (epoll_socket_ == -1) {
        throw std::runtime_error{"epoll_create1() failed"};
    }

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.ptr = nullptr;

    if (epoll_ctl(epoll_socket_, EPOLL_CTL_ADD, server_socket_.descriptor(), &ev) == -1) {
        throw std::runtime_error("epoll_ctl() failed");
    }
}

void async_server_base::run()
{
    while (true) {
        run_once();
    }
}

void async_server_base::run_once()
{
    int number = epoll_wait(epoll_socket_, events_, MAX_EVENTS, -1);

    for (int i = 0; i < number; ++i) {
        auto const& d = events_[i].data;

        if (d.ptr == nullptr) {
            accept_new_client();
        } else {
            call_ready(d.u32, get_socket(d.u32));
        }
    }
}

void async_server_base::accept_new_client()
{
    if (!has_space()) {
        return;
    }

    auto id = add_client(server_socket_.accept());
    auto& sck = get_socket(id);
    call_accepted(sck);

    epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.u32 = id;

    if (epoll_ctl(epoll_socket_, EPOLL_CTL_ADD, sck.descriptor(), &ev) == -1) {
        throw std::runtime_error("epoll_ctl() failed");
    }
}

}