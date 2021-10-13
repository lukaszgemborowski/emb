#include "emb/net/uds_client_socket.hpp"
#include <sys/socket.h>
#include <sys/un.h>
#include <stdexcept>

namespace emb::net
{

uds_client_socket::uds_client_socket(std::string_view name)
    : socket(AF_UNIX, SOCK_STREAM, 0)
{
    connect(name);
}

void uds_client_socket::connect(std::string_view name) const
{
    sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, name.data(), sizeof(addr.sun_path) - 1);

    if(::connect(descriptor(), (sockaddr *)&addr, sizeof(addr)) == -1) {
        throw std::runtime_error{"connect() failed"};
    }
}

}