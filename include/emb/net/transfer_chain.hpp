#ifndef EMB_RPC_TRANSFER_CHAIN_HPP
#define EMB_RPC_TRANSFER_CHAIN_HPP

#include <emb/net/async_socket_server.hpp>
#include <tuple>

namespace emb::net
{

namespace detail
{
template<class Func>
struct chain_read {
    chain_read(std::size_t count, Func func)
        : count_ {count}
        , func_ {func}
    {}

private:
    std::size_t count_;
    Func func_;
};
}

template<class RWBuffer, class... Seq>
class transfer_chain
{
public:
    transfer_chain(async_socket_server& async, RWBuffer& buffer)
        : async_ {async}
        , buffer_ {buffer}
    {}

    template<class Func>
    auto read(std::size_t count, Func func) {
        return transfer_chain<RWBuffer, Seq..., chain_read>
    }

private:
    async_socket_server& async_;
    RWBuffer& buffer_;
};

}

#endif // EMB_RPC_TRANSFER_CHAIN_HPP