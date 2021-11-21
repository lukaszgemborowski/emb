#ifndef EMB_RPC_SOCKET_CLIENT_HPP
#define EMB_RPC_SOCKET_CLIENT_HPP

#include <emb/net/socket.hpp>
#include <emb/rpc/api.hpp>
#include <emb/ser/serialize.hpp>
#include <emb/ser/deserialize.hpp>
#include <emb/contiguous_buffer.hpp>
#include <array>
#include <stdexcept>

namespace emb::rpc
{

class socket_client_base
{
public:
    explicit socket_client_base(emb::net::socket& sck);
    virtual ~socket_client_base() = default;

protected:
    void send_call(emb::contiguous_buffer<unsigned char> buffer) const;
    void read_response(emb::contiguous_buffer<unsigned char> destination) const;

private:
    emb::net::socket&   socket_;
};

template<class Api>
class socket_client : public socket_client_base
{
public:
    socket_client(emb::net::socket& sck)
        : socket_client_base {sck}
    {}

    template<auto Id, class... Args>
    auto call(Args... args) {
        // TODO: get rid of this temporary tuple, eg. pass args... directly to serialize() with API type checking
        using types = decltype(Api{}.template get_signature<Id>().get_types());
        auto payload = std::tuple_cat(std::tuple<int>{static_cast<int>(Id)}, typename types::arguments_tuple{args...});

        if (auto size = ser::serialize(payload, buffer_); size >= 0) {
            send_call({buffer_.data(), size});
        }

        if constexpr (std::is_same_v<typename types::return_type, void> == false) {
            read_response({buffer_.data(), buffer_.size()});
            std::tuple<typename types::return_type> resp;
            if (ser::deserialize(resp, buffer_)) {
                return std::get<0>(resp);
            } else {
                throw std::runtime_error{"rpc::call failed to deserialize response"};
            }
        }
    }

private:
    // TODO: make it a template parameter
    std::array<unsigned char, 512>  buffer_;
};

template<class Api>
auto make_socket_client(Api, emb::net::socket& sck) {
    return socket_client<Api>{sck};
}

}

#endif // EMB_RPC_SOCKET_CLIENT_HPP