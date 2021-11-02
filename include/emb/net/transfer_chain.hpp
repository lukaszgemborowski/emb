#ifndef EMB_RPC_TRANSFER_CHAIN_HPP
#define EMB_RPC_TRANSFER_CHAIN_HPP

#include <emb/net/async_socket_server.hpp>
#include <emb/call_chain.hpp>
#include <functional>
#include <tuple>

namespace emb::net
{

namespace transfer_chain
{
namespace detail
{
template<class Buffer>
struct context
{
    Buffer&                 buffer;
    async_socket_server&    async;
    async_core::node_id     node;
};
}

class read_unconditional
{
public:
    read_unconditional(std::size_t count)
        : count_ {count}
    {}

    template<class Context, class Func>
    void execute(Context& ctx, Func next) {
        ctx.async.read(
            ctx.node,
            emb::contiguous_buffer{ctx.buffer, count_},
            next
        );
    }

private:
    std::size_t count_;
};

class read_conditional
{
public:
    using func_type = std::function<std::size_t (emb::contiguous_buffer<unsigned char>)>;

    read_conditional(func_type func)
        : func_ {std::move(func)}
    {}

    template<class Context, class Func>
    void execute(Context& ctx, Func next) {
        auto const bytes_to_read = func_(emb::contiguous_buffer<unsigned char>{ctx.buffer});

        ctx.async.read(
            ctx.node,
            emb::contiguous_buffer{ctx.buffer, bytes_to_read},
            next
        );
    }

private:
    func_type func_;
};

class process_data
{
public:
    using func_type = std::function<void (emb::contiguous_buffer<unsigned char>)>;

    process_data(func_type func)
        : func_ {std::move(func)}
    {}

    template<class Context, class Func>
    void execute(Context& ctx, Func next) {
        func_(emb::contiguous_buffer<unsigned char>{ctx.buffer});
    }

private:
    func_type func_;
};

inline auto read(std::size_t count) {
    return read_unconditional{count};
}

inline auto read(read_conditional::func_type func) {
    return read_conditional(func);
}

inline auto process(process_data::func_type func) {
    return process_data{func};
}

template<class RWBuffer, class... Seq>
class root
{
public:
    root(async_socket_server& async, async_core::node_id node, RWBuffer& buffer)
        : ctx_ {buffer, async, node}
    {}

    template<class... S>
    root(detail::context<RWBuffer> const& ctx, std::tuple<S...>&& s)
        : ctx_ {ctx}
        , sequence_ {std::move(s)}
    {}

    template<class S>
    auto append(S&& node) {
        return root<RWBuffer, Seq..., S>{
            ctx_,
            std::tuple_cat(std::move(sequence_), std::tuple<S>{std::move(node)})
        };
    }

    template<class S>
    auto operator>>(S&& node) {
        return append(std::move(node));
    }

    void run(async_core::node_id node) {
        ctx_.node = node;
        execute<0>();
    }

private:
    template<std::size_t I>
    void execute() {
        auto& step = std::get<I>(sequence_);
        step.execute(
            ctx_, [&] {
                if constexpr (I < sizeof...(Seq) - 1) {
                    execute<I + 1>();
                }
            }
        );
    }


private:
    detail::context<RWBuffer>   ctx_;
    std::tuple<Seq...>          sequence_;
};

}

}

#endif // EMB_RPC_TRANSFER_CHAIN_HPP