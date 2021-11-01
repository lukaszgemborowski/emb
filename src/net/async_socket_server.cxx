#include "emb/net/async_socket_server.hpp"
#include <algorithm>

namespace emb::net
{
namespace
{
constexpr auto SERVER_ID = 1;

}

async_core::node_id async_socket_server::next_id_ = SERVER_ID + 1;

async_socket_server::async_socket_server(socket& sck)
    : server_socket_ {sck}
{
    async_.on_event = [this](auto id, auto flags) { socket_event(id, flags);};

    async_.add(
        SERVER_ID,
        server_socket_,
        async_watch_flags{async_watch::in, async_watch::hup, async_watch::err}
    );
}

async_socket_server::~async_socket_server()
{
    async_.remove(server_socket_);
}

void async_socket_server::run_once()
{
    async_.run_once();
}

void async_socket_server::write(async_core::node_id id, detail::write_buffer buffer, detail::write_data::callback_type func)
{
    if (buffer.size() == 0) {
        func();
        return;
    }

    auto entry = client_by_id(id);
    if (entry == clients_.end()) {
        return;
    }

    entry->pending_write.buffer = buffer;
    entry->pending_write.callback = func;
    entry->flags.set<async_watch::out>();
    async_.modify(id, entry->sock, entry->flags);
}

void async_socket_server::read(async_core::node_id id, detail::read_buffer buffer, detail::read_data::callback_type func)
{
    if (buffer.size() == 0) {
        func();
        return;
    }

    auto entry = client_by_id(id);
    if (entry == clients_.end()) {
        return;
    }

    entry->pending_read.buffer = buffer;
    entry->pending_read.callback = func;
    entry->flags.set<async_watch::in>();
    async_.modify(id, entry->sock, entry->flags);
}

void async_socket_server::socket_event(async_core::node_id id, async_watch_flags ev)
{
    if (id == SERVER_ID) {
        process_server_event(ev);
    } else {
        auto entry = client_by_id(id);
        if (entry == clients_.end()) {
            return;
        }

        if (ev.is<async_watch::out>()) {
            process_out_event(*entry);
        }

        if (ev.is<async_watch::in>()) {
            process_in_event(*entry);
        }

        if (ev.is<async_watch::hup>()) {
            process_hup_event(*entry);
        }
    }
}

void async_socket_server::process_server_event(async_watch_flags ev)
{
    if (ev.is<async_watch::in>()) {
        clients_.emplace_back(next_id_, server_socket_.accept());
        ++next_id_;
        auto& ops = clients_.back();
        async_.modify(SERVER_ID, server_socket_, async_watch_flags{async_watch::in, async_watch::hup, async_watch::err});
        async_.add(ops.id, ops.sock, async_watch_flags{});
        on_accept_cb.call(ops.id);
    }
}

void async_socket_server::process_out_event(detail::async_socket_operations& op)
{
    auto &pw = op.pending_write;

    auto r = op.sock.write_some(pw.buffer);

    if (r < pw.buffer.size()) {
        pw.buffer = pw.buffer.slice(r, npos);
    } else {
        op.flags.clear<async_watch::out>();
        async_.modify(op.id, op.sock, op.flags);
        op.pending_write.callback();
    }
}

void async_socket_server::process_in_event(detail::async_socket_operations& op)
{
    auto &pw = op.pending_read;

    auto r = op.sock.read_some(pw.buffer);

    if (r < pw.buffer.size()) {
        pw.buffer = pw.buffer.slice(r, npos);
    } else {
        op.flags.clear<async_watch::in>();
        async_.modify(op.id, op.sock, op.flags);
        op.pending_read.callback();
    }
}

void async_socket_server::process_hup_event(detail::async_socket_operations& op)
{
    if (on_disconnect_cb.is_set()) {
        on_disconnect_cb.call(op.id);
    }

    async_.remove(op.sock);
    clients_.erase(
        std::remove_if(
            clients_.begin(),
            clients_.end(),
            [&] (auto const& e) {
                return e.id == op.id;
            }
        ),
        clients_.end()
    );
}

std::vector<detail::async_socket_operations>::iterator async_socket_server::client_by_id(async_core::node_id id)
{
    return std::find_if(
        clients_.begin(),
        clients_.end(),
        [id](auto const& e) { return e.id == id; }
    );
}

std::size_t async_socket_server::clients_count() const
{
    return clients_.size();
}

}