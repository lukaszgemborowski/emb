#include "emb/rpc/socket_client.hpp"


#include <iostream>

namespace emb::rpc
{

socket_client_base::socket_client_base(emb::net::socket& sck)
    : socket_ {sck}
{
}

void socket_client_base::send_call(emb::contiguous_buffer<unsigned char> buffer) const
{
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