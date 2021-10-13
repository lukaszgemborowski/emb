#ifndef EMB_RPC_API_HPP
#define EMB_RPC_API_HPP

#include <tuple>
#include <emb/rpc/procedure.hpp>

namespace emb::rpc
{

template<typename... Functions>
struct api {
    template<auto Id>
    constexpr auto get_signature() const {
        return get_sig_impl<Id, Functions...>();
    }

    struct none {};

    template<auto Id, class F, class... T>
    constexpr auto get_sig_impl() const {
        if constexpr (F::template compare_id<Id>()) {
            return F{};
        } else {
            if constexpr (sizeof... (T) == 0) {
                return none {};
            } else {
                return get_sig_impl<Id, T...>();
            }
        }
    }

    template<class F>
    constexpr auto transform() const {
        return transform_impl<F, Functions...>();
    }

    template<class F, class... U>
    constexpr auto transform_impl() const {
        return transformation<typename F::template type<U>...>{};
    }

    template<class... T>
    struct transformation {
        using as_tuple = std::tuple<T...>;
    };
};

template<typename... Functions>
constexpr auto make_api(Functions...) {
    return api<Functions...>{};
}

}

#endif // EMB_RPC_API_HPP