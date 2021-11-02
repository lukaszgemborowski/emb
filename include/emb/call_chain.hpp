#ifndef EMB_CALL_CHAIN_HPP
#define EMB_CALL_CHAIN_HPP

#include <tuple>
#include <type_traits>

namespace emb
{

template<class... Seq>
class call_chain
{
public:
    explicit call_chain(Seq&&... seq)
        : sequence_ {std::forward<Seq>(seq)...}
    {}

    explicit call_chain(std::tuple<Seq...>&& tup)
        : sequence_ {std::move(tup)}
    {}

    void step() {
        step_impl(std::make_index_sequence<sizeof...(Seq)>{});
        ++current_;
    }

    bool finished() const {
        return current_ == sizeof...(Seq);
    }

    void reset() & {
        current_ = 0;
    }

    template<class S>
    auto append(S&& fun) & {
        return call_chain<Seq..., S>{ std::tuple_cat(sequence_, std::tuple<S>{std::forward<S>(fun)}) };
    }

    template<class S>
    auto append(S&& fun) && {
        return call_chain<Seq..., S>{ std::tuple_cat(std::move(sequence_), std::tuple<S>{std::forward<S>(fun)}) };
    }

private:
    template<std::size_t C, std::size_t... I>
    void step_impl(std::index_sequence<C, I...>) {
        if (C == current_) {
            std::get<C>(sequence_)();
            return;
        }

        step_impl(std::index_sequence<I...>{});
    }

    void step_impl(std::index_sequence<>) {}

private:
    std::tuple<Seq...>  sequence_;
    std::size_t         current_ = 0;
};

}

#endif // EMB_CALL_CHAIN_HPP