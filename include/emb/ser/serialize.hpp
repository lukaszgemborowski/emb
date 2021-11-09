#ifndef EMB_SER_SERIALIZE_HPP
#define EMB_SER_SERIALIZE_HPP

#include <emb/contiguous_buffer.hpp>
#include <tuple>
#include <type_traits>
#include <cstring>
#include <array>

namespace emb::ser
{

using tuple_size_type = std::uint32_t;
constexpr tuple_size_type invalid_size = 0xffffffff;

namespace detail
{

template<class T>
struct serdes {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

    static bool serialize(T const& object, unsigned char** curr, unsigned char* end) {
        if (end - *curr < sizeof(T)) {
            return false;
        }

        std::memcpy(*curr, &object, sizeof(object));
        (*curr) += sizeof(T);
        return true;
    }

    static bool deserialize(T& object, unsigned char const** curr, unsigned char const* end) {
        if (end - *curr < sizeof(T)) {
            return false;
        }

        std::memcpy(&object, *curr, sizeof(T));
        (*curr) += sizeof(T);
        return true;
    }

    static constexpr auto min_size() {
        return sizeof(T);
    }

    static constexpr auto max_size() {
        return sizeof(T);
    }

    static constexpr auto is_constant_size() {
        return true;
    }

    using element_type = T;
};

template<class T, std::size_t N>
struct serdes<std::array<T, N>> {
    static bool serialize(std::array<T, N> const& object, unsigned char** curr, unsigned char* end) {
        if (end - *curr  < min_size()) {
            return false;
        }

        for (std::size_t i = 0; i < N; ++i) {
            if (serdes<T>::serialize(object[i], curr, end) == false) {
                return false;
            }
        }

        return true;
    }

    static bool deserialize(std::array<T, N>& object, unsigned char const** curr, unsigned char const* end) {
        if (end - *curr < min_size()) {
            return false;
        }

        for (std::size_t i = 0; i < N; ++i) {
            if (serdes<T>::deserialize(object[i], curr, end) == false) {
                return false;
            }
        }

        return true;
    }

    static constexpr auto min_size() {
        return serdes<T>::min_size() * N;
    }

    static constexpr auto max_size() {
        return serdes<T>::min_size() * N;
    }

    static constexpr auto is_constant_size() {
        return serdes<T>::is_constant_size();
    }

    using element_type = std::array<T, N>;
};

template<>
struct serdes<std::string>
{
    using size_type = std::uint16_t;
    using size_type_serializer = serdes<size_type>;

    static_assert(size_type_serializer::is_constant_size());

    static bool serialize(std::string const& object, unsigned char** curr, unsigned char* end) {
        if (object.size() > std::numeric_limits<size_type>::max()) {
            return false;
        }

        if (end - *curr  < object.size() + size_type_serializer::min_size()) {
            return false;
        }

        size_type size = static_cast<size_type>(object.size());

        if (!size_type_serializer::serialize(size, curr, end)) {
            return false;
        }

        std::memcpy(*curr, object.c_str(), size);
        *curr += size;

        return true;
    }

    static bool deserialize(std::string& object, unsigned char const** curr, unsigned char const* end) {
        if (end - *curr < size_type_serializer::min_size()) {
            return false;
        }

        size_type size = 0;

        if (!size_type_serializer::deserialize(size, curr, end)) {
            return false;
        }

        if (end - *curr < size) {
            return false;
        }

        object = std::string{reinterpret_cast<const char*>(*curr), size};
        *curr += size;

        return true;
    }

    static constexpr auto min_size() {
        return 0;
    }

    static constexpr auto max_size() {
        return std::numeric_limits<size_type>::max();
    }

    static constexpr auto is_constant_size() {
        return false;
    }

    using element_type = std::string;
};

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

template<class T>
std::size_t serialize(T const& tuple, unsigned char* begin, unsigned char* end)
{
    auto *curr = begin;
    bool result = serdes<T>::serialize(tuple, &curr, end);

    if (result) {
        return curr - begin;
    } else {
        return -1;
    }
}

template<class T>
std::size_t deserialize(T& tuple, unsigned char const* begin, unsigned char const* end)
{
    auto *curr = begin;
    bool result = serdes<T>::deserialize(tuple, &curr, end);

    if (result) {
        return curr - begin;
    } else {
        return -1;
    }
}

}

// size query interface
template<class... T> constexpr auto size_requirements(std::tuple<T...> const&)
{
    struct result {
        std::size_t max, min;
    };

    result r;
    r.min = detail::serdes<std::tuple<T...>>::min_size();
    r.max = detail::serdes<std::tuple<T...>>::max_size();

    return r;
}

inline auto deserialize_size(emb::contiguous_buffer<const unsigned char> buffer)
{
    if (buffer.size() < sizeof(std::uint32_t)) {
        return invalid_size;
    }

    tuple_size_type size = 0;
    auto ptr = buffer.data();
    auto pptr = &ptr;
    if (detail::serdes<tuple_size_type>::deserialize(size, pptr, buffer.data() + buffer.size())) {
        return size + static_cast<tuple_size_type>(sizeof(tuple_size_type));
    } else {
        return invalid_size;
    }
}

// serialize interface
template<class... T> auto serialize(std::tuple<T...> const& tuple, unsigned char* begin, unsigned char* end)
{
    return detail::serialize(tuple, begin, end);
}

template<std::size_t N, class... T> auto serialize(std::tuple<T...> const& tuple, std::array<unsigned char, N>& dest)
{
    return detail::serialize(tuple, dest.data(), dest.data() + dest.size());
}

template<class... T> auto serialize(std::tuple<T...> const& tuple, emb::contiguous_buffer<unsigned char>&& dest)
{
    return detail::serialize(tuple, dest.data(), dest.data() + dest.size());
}

// deserialize interface
template<class... T> auto deserialize(std::tuple<T...>& tuple, unsigned char* begin, unsigned char* end)
{
    return detail::deserialize(tuple, begin, end);
}

template<std::size_t N, class... T> auto deserialize(std::tuple<T...>& tuple, std::array<unsigned char, N> const& src)
{
    return detail::deserialize(tuple, src.data(), src.data() + src.size());
}

template<class... T> auto deserialize(std::tuple<T...>& tuple, emb::contiguous_buffer<unsigned char>&& src)
{
    return detail::deserialize(tuple, src.data(), src.data() + src.size());
}

}

#endif // EMB_SER_SERIALIZE_HPP