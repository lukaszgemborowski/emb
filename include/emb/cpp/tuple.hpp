#ifndef EMB_CPP_TUPLE_HPP
#define EMB_CPP_TUPLE_HPP

#include <tuple>

namespace emb::cpp
{

namespace detail
{

template<std::size_t Offset, std::size_t... I>
constexpr auto offset_index_sequence(std::index_sequence<I...>) {
    return std::index_sequence<(I + Offset)...>{};
}

template<std::size_t... I, class... T>
constexpr auto tuple_slice(std::tuple<T...> const& src, std::index_sequence<I...>) {
    return std::tuple<
        std::tuple_element_t<I, std::tuple<T...>>...> {std::get<I>(src)...};
}

}

/**
 * \brief Slice the tuple, create a sub-tuple.
 *
 * Slice C items starting at index S. Create a new tuple with a copy
 * of the elements [S, C+S].
 */
template<std::size_t S, std::size_t C, class... T>
constexpr auto tuple_slice(std::tuple<T...> const& src) {
    static_assert(S >= 0                , "start index must be equal or greater than 0");
    static_assert(C >= 0                , "count must be equal or greater than 0");
    static_assert(S + C <= sizeof...(T) , "range is outside of the tuple");

    if constexpr (C == 0) {
        return std::tuple<>{};
    } else {
        return detail::tuple_slice(
            src,
            detail::offset_index_sequence<S>(
                std::make_index_sequence<C>{}
            )
        );
    }
}

}

#endif // EMB_CPP_TUPLE_HPP