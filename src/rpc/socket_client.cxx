#include "emb/rpc/socket_client.hpp"

namespace emb::rpc
{

socket_client_base::socket_client_base(emb::net::socket& sck)
    : socket_ {sck}
{
}

void socket_client_base::send_call(int call_id, emb::contiguous_buffer<unsigned char> buffer) const
{
    socket_.write(call_id);
    socket_.write(buffer.size());
    socket_.write(buffer.data(), buffer.size());
}

void socket_client_base::read_response(emb::contiguous_buffer<unsigned char> destination) const
{
    auto size = socket_.read<std::size_t>();
    if (size <= destination.size()) {
        socket_.read(destination, size);
    }
}

}