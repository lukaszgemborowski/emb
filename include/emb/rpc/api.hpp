#ifndef EMB_RPC_API_HPP
#define EMB_RPC_API_HPP

#include <tuple>
#include <emb/rpc/procedure.hpp>

namespace emb::rpc
{
namespace detail
{
template<class F, class... T> struct first_type {
    using type = F;
};
struct none {};
}

template<typename... Functions>
struct api {
    // detail
    using d_first_function_type = typename detail::first_type<Functions...>::type;
    using d_first_function_id_type = typename d_first_function_type::id_type;

    static_assert(
        (std::is_same_v<d_first_function_id_type, typename Functions::id_type> && ...),
        "Every procedure ID need to have the same type"
    );
    // detail

    template<auto Id>
    constexpr auto get_signature() const {
        return get_sig_impl<Id, Functions...>();
    }

    template<auto Id, class F, class... T>
    constexpr auto get_sig_impl() const {
        if constexpr (F::template compare_id<Id>()) {
            return F{};
        } else {
            if constexpr (sizeof... (T) == 0) {
                return detail::none {};
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