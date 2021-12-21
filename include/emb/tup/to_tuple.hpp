#ifndef EMB_TUP_TO_TUPLE_HPP
#define EMB_TUP_TO_TUPLE_HPP

#include <type_traits>
#include <utility>
#include <tuple>

namespace emb::tup
{
namespace detail
{

template<std::size_t>
struct convertible_to_any {
    template<class T>
    constexpr operator T();
};

template<class T, std::size_t... Idx>
constexpr auto ctor_with_n_args_impl(std::index_sequence<Idx...>) -> decltype(T{convertible_to_any<Idx>{}...});

template<class T, std::size_t N>
constexpr decltype(ctor_with_n_args_impl<T>(std::make_index_sequence<N>{})) ctor_with_n_args();

template<class, std::size_t>
std::false_type
constructible_with_n(...);

template<class T, std::size_t N>
decltype(ctor_with_n_args<T, N>(), std::true_type{})
constructible_with_n(int);

template<class T, std::size_t N = 1, bool C = true>
struct args_constructible
{
    static constexpr auto constructible = decltype(constructible_with_n<T, N>(0))::value;

    static constexpr auto value = constructible
        ? args_constructible<T, N+1, constructible>::value
        : N-1;
};

template<class T, std::size_t N>
struct args_constructible<T, N, false> {
    static constexpr auto value = -1;
};

template<class T, std::size_t N>
struct to_tuple;

template<class T> struct to_tuple<T, 1> {
    static auto convert(T const& in) {
        auto [a0] = in;
        return std::make_tuple(a0);
    }
};
template<class T> struct to_tuple<T, 2> {
    static auto convert(T const& in) {
        auto [a0, a1] = in;
        return std::make_tuple(a0, a1);
    }
};
template<class T> struct to_tuple<T, 3> {
    static auto convert(T const& in) {
        auto [a0, a1, a2] = in;
        return std::make_tuple(a0, a1, a2);
    }
};

} // namespace detail

template<class T>
auto to_tuple(T const& in) {
    return detail::to_tuple<T, detail::args_constructible<T>::value>::convert(in);
}

} // namespace emb::tup

#endif // EMB_TUP_TO_TUPLE_HPP