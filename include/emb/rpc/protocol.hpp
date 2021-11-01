#ifndef EMB_RPC_PROTOCOL_HPP
#define EMB_RPC_PROTOCOL_HPP

#include <tuple>

namespace emb::rpc::protocol
{
using request_header = std::tuple<int, std::size_t>;
using response_header = std::tuple<std::size_t>;
}

#endif // EMB_RPC_PROTOCOL_HPP