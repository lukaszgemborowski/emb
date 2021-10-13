#ifndef EMB_NET_UDS_CLIENT_SOCKET_HPP
#define EMB_NET_UDS_CLIENT_SOCKET_HPP

#include <emb/net/socket.hpp>
#include <string_view>

namespace emb::net
{

class uds_client_socket final : public socket
{
public:
    explicit uds_client_socket(std::string_view name);

private:
    void connect(std::string_view name) const;
};

}

#endif // EMB_NET_UDS_CLIENT_SOCKET_HPP