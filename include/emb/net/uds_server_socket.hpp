#ifndef EMB_NET_UDS_SERVER_SOCKET_HPP
#define EMB_NET_UDS_SERVER_SOCKET_HPP

#include <emb/net/socket.hpp>
#include <string_view>

namespace emb::net
{

class uds_server_socket final : public socket
{
public:
    explicit uds_server_socket(std::string_view name);

private:
    void bind_and_listen(std::string_view name) const;
};

}

#endif // EMB_NET_UDS_SERVER_SOCKET_HPP