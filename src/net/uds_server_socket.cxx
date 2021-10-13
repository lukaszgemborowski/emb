#include "emb/net/uds_server_socket.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdexcept>

namespace emb::net
{

uds_server_socket::uds_server_socket(std::string_view name)
    : socket(AF_UNIX, SOCK_STREAM, 0)
{
    bind_and_listen(name);
}

void uds_server_socket::bind_and_listen(std::string_view name) const
{
    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, name.data(), sizeof(addr.sun_path) - 1);

    ::unlink(name.data());
    if(::bind(descriptor(), (sockaddr *)&addr, sizeof(addr)) == -1) {
        throw std::runtime_error{"bind() failed"};
    }

    if (::listen(descriptor(), 10) == -1) {
        throw std::runtime_error{"listen() failed"};
    }
}

}