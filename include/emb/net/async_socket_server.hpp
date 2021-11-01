#ifndef EMB_NET_ASYNC_SOCKET_SERVER_HPP
#define EMB_NET_ASYNC_SOCKET_SERVER_HPP

#include <emb/net/async_core.hpp>
#include <emb/contiguous_buffer.hpp>
#include <string_view>

namespace emb::net
{

namespace detail
{

using write_buffer = contiguous_buffer<const unsigned char>;
using read_buffer = contiguous_buffer<unsigned char>;

struct write_data
{
    using callback_type = std::function<void ()>;
    callback_type   callback;
    write_buffer    buffer;
};

struct read_data
{
    using callback_type = std::function<void ()>;
    callback_type   callback;
    read_buffer     buffer;
};

struct async_socket_operations
{
    async_socket_operations(async_core::node_id id, socket&& sock)
        : id {id}
        , sock {std::move(sock)}
    {}

    async_core::node_id     id;
    socket                  sock;
    async_watch_flags       flags;
    write_data              pending_write;
    read_data               pending_read;
};

}

class async_socket_server
{
    callback<void (async_core::node_id)> on_accept_cb;
    callback<void (async_core::node_id)> on_disconnect_cb;
public:
    using accept_function = std::function<void (async_core::node_id)>;

    explicit async_socket_server(socket& sck);
    ~async_socket_server();

    void write(async_core::node_id id, detail::write_buffer buffer, detail::write_data::callback_type func);
    void read(async_core::node_id id, detail::read_buffer buffer, detail::read_data::callback_type func);

    void run_once();

    std::size_t clients_count() const;

    public_callback<void (async_core::node_id)> on_accept {on_accept_cb};
    public_callback<void (async_core::node_id)> on_disconnect {on_disconnect_cb};

private:
    void socket_event(async_core::node_id id, async_watch_flags ev);
    void process_out_event(detail::async_socket_operations& op);
    void process_in_event(detail::async_socket_operations& op);
    void process_hup_event(detail::async_socket_operations& op);
    void process_server_event(async_watch_flags ev);

    std::vector<detail::async_socket_operations>::iterator client_by_id(async_core::node_id id);

private:
    socket& server_socket_;
    async_core async_;
    accept_function accept_func_;
    std::vector<detail::async_socket_operations> clients_;

    static async_core::node_id next_id_;
};

}

#endif // EMB_NET_ASYNC_SOCKET_SERVER_HPP