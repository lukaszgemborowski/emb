#ifndef EMB_SER_SERDES_TUPLE_HPP
#define EMB_SER_SERDES_TUPLE_HPP

#include <emb/ser/serdes/serdes.hpp>
#include <tuple>
#include <type_traits>

namespace emb::ser::detail
{

template<class... T>
struct serdes<std::tuple<T...>>
{
    static bool serialize(std::tuple<T...> const& object, unsigned char** curr, unsigned char* end) {
        std::uint32_t overall_size = 0;
        bool have_space = true;
        auto buffer_begin = *curr;
        *curr += sizeof(overall_size);

        visit_all(
            [&](auto const& value) {
                using type = std::remove_cv_t<std::remove_reference_t<decltype(value)>>;
                if (have_space && !serdes<type>::serialize(value, curr, end)) {
                    have_space = false;
                }
            },
            object,
            std::make_index_sequence<sizeof...(T)>{}
        );

        if (!have_space) {
            return false;
        }

        overall_size = *curr - buffer_begin - sizeof(overall_size);
        serdes<std::uint32_t>::serialize(overall_size, &buffer_begin, end);

        return true;
    }

    static bool deserialize(std::tuple<T...>& object, unsigned char const** curr, unsigned char const* end) {
        std::uint32_t overall_size = -1;
        bool correct = true;
        if (!serdes<std::uint32_t>::deserialize(overall_size, curr, end)) {
            return false;
        }

        visit_all(
            [&](auto& value) {
                using type = std::remove_reference_t<std::remove_const_t<decltype(value)>>;
                if (correct && !serdes<type>::deserialize(value, curr, end)) {
                    correct = false;
                }
            },
            object,
            std::make_index_sequence<sizeof...(T)>{}
        );

        return correct;
    }

    static constexpr auto min_size() {
        return min_size_all(std::make_index_sequence<sizeof...(T)>{}) + sizeof(std::uint32_t);
    }

    static constexpr auto max_size() {
        return max_size_all(std::make_index_sequence<sizeof...(T)>{}) + sizeof(std::uint32_t);
    }

private:
    template<std::size_t... I>
    static constexpr auto min_size_all(std::index_sequence<I...>) {
        return (serdes<std::tuple_element_t<I, std::tuple<T...>>>::min_size() + ...);
    }

    template<std::size_t... I>
    static constexpr auto max_size_all(std::index_sequence<I...>) {
        return (serdes<std::tuple_element_t<I, std::tuple<T...>>>::max_size() + ...);
    }

    template<class Func, class Tuple, std::size_t C, std::size_t... I>
    static constexpr auto visit_all(Func func, Tuple& tuple, std::index_sequence<C, I...>) {
        func(std::get<C>(tuple));
        visit_all(func, tuple, std::index_sequence<I...>{});
    }

    template<class Func, class Tuple>
    static constexpr auto visit_all(Func, Tuple&, std::index_sequence<>) {}
};

} // namespace emb::ser::detail

#endif // EMB_SER_SERDES_TUPLE_HPP