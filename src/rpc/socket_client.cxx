#include "emb/rpc/socket_client.hpp"

namespace emb::rpc
{

socket_client_base::socket_client_base(emb::net::socket& sck)
    : socket_ {sck}
{
}

void socket_client_base::send_call(int call_id, emb::contiguous_buffer<unsigned char> buffer) const
{
    static_assert(sizeof(call_id) < sizeof(buffer.size()));

    std::array<unsigned char, sizeof(buffer.size())> int_buffer;
    auto buffer_size = buffer.size();

    std::memcpy(int_buffer.data(), &call_id, sizeof(call_id));
    socket_.write_some(emb::contiguous_buffer{int_buffer, sizeof(call_id)});

    std::memcpy(int_buffer.data(), &buffer_size, sizeof(buffer_size));
    socket_.write_some(emb::contiguous_buffer{int_buffer, sizeof(buffer_size)});
    socket_.write_some(buffer);
}

void socket_client_base::read_response(emb::contiguous_buffer<unsigned char> destination) const
{
    std::size_t size = 0;
    std::array<unsigned char, sizeof(size)> buff;
    socket_.read(emb::contiguous_buffer{buff});
    std::memcpy(&size, buff.data(), sizeof(size));

    if (size <= destination.size()) {
        socket_.read(destination.slice(0, size));
    }
}

}