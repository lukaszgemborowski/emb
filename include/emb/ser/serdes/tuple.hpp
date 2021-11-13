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
    using encoded_size_type = std::uint32_t;

    static std::size_t serialize(std::tuple<T...> const& object, unsigned char* curr, unsigned char* end) {
        encoded_size_type overall_size = 0;
        bool have_space = true;

        visit_all(
            [&](auto const& value) {
                using type = std::remove_cv_t<std::remove_reference_t<decltype(value)>>;

                if (!have_space) {
                    return;
                }

                if (auto size = serdes<type>::serialize(value, curr + overall_size + sizeof(encoded_size_type), end); size >= 0) {
                    overall_size += size;
                } else {
                    have_space = false;
                }
            },
            object,
            std::make_index_sequence<sizeof...(T)>{}
        );

        if (!have_space) {
            return false;
        }

        serdes<std::uint32_t>::serialize(overall_size, curr, end);

        return overall_size + sizeof(encoded_size_type);
    }

    static std::size_t deserialize(std::tuple<T...>& object, unsigned char const* curr, unsigned char const* end) {
        encoded_size_type overall_size = 0;
        bool correct = true;
        auto offset = serdes<std::uint32_t>::deserialize(overall_size, curr, end);

        if (offset == -1) {
            return false;
        }

        visit_all(
            [&](auto& value) {
                using type = std::remove_reference_t<std::remove_const_t<decltype(value)>>;

                if (!correct) {
                    return;
                }

                if (auto size = serdes<type>::deserialize(value, curr + offset, end); size >= 0) {
                    offset += size;
                } else {
                    correct = false;
                }
            },
            object,
            std::make_index_sequence<sizeof...(T)>{}
        );

        return offset;
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