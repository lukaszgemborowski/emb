#ifndef EMB_SER_SERIALIZE_HPP
#define EMB_SER_SERIALIZE_HPP

#include <emb/contiguous_buffer.hpp>
#include <tuple>
#include <type_traits>
#include <cstring>
#include <array>

namespace emb::ser
{
namespace detail
{

template<class T>
struct serdes {
    static constexpr auto min_size_required = sizeof(T);
    static constexpr auto max_size_required = sizeof(T);

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
};

template<class T> bool serialize_single(T const& object, unsigned char** curr, unsigned char* end)
{
    return serdes<T>::serialize(object, curr, end);
}

template<class T> bool deserialize_single(T& object, unsigned char const** curr, unsigned char const* end)
{
    return serdes<T>::deserialize(object, curr, end);
}

template<class T, std::size_t... I>
std::size_t serialize(T const& tuple, unsigned char* begin, unsigned char* end, std::index_sequence<I...>)
{
    auto *curr = begin;
    bool result = (serialize_single(std::get<I>(tuple), &curr, end) && ...);
    if (result) {
        return curr - begin;
    } else {
        return -1;
    }
}

template<class T, std::size_t... I>
std::size_t deserialize(T& tuple, unsigned char const* begin, unsigned char const* end, std::index_sequence<I...>)
{
    auto *curr = begin;
    bool result = (deserialize_single(std::get<I>(tuple), &curr, end) && ...);
    if (result) {
        return curr - begin;
    } else {
        return -1;
    }
}

template<class T, std::size_t... I>
constexpr auto size_requirements(std::index_sequence<I...>)
{
    struct result {
        std::size_t max = 0, min = 0;
    };

    result r;

    r.min = (serdes<std::tuple_element_t<I, T>>::min_size_required + ...);
    r.max = (serdes<std::tuple_element_t<I, T>>::max_size_required + ...);

    return r;
}

}

// size query interface
template<class... T> constexpr auto size_requirements(std::tuple<T...> const&)
{
    return detail::size_requirements<std::tuple<T...>>(std::make_index_sequence<sizeof...(T)>{});
}

// serialize interface
template<class... T> auto serialize(std::tuple<T...> const& tuple, unsigned char* begin, unsigned char* end)
{
    return detail::serialize(tuple, begin, end, std::make_index_sequence<sizeof...(T)>{});
}

template<std::size_t N, class... T> auto serialize(std::tuple<T...> const& tuple, std::array<unsigned char, N>& dest)
{
    return detail::serialize(tuple, dest.data(), dest.data() + dest.size(), std::make_index_sequence<sizeof...(T)>{});
}

template<class... T> auto serialize(std::tuple<T...> const& tuple, emb::contiguous_buffer<unsigned char>&& dest)
{
    return detail::serialize(tuple, dest.data(), dest.data() + dest.size(), std::make_index_sequence<sizeof...(T)>{});
}

// deserialize interface
template<class... T> auto deserialize(std::tuple<T...>& tuple, unsigned char* begin, unsigned char* end)
{
    return detail::deserialize(tuple, begin, end, std::make_index_sequence<sizeof...(T)>{});
}

template<std::size_t N, class... T> auto deserialize(std::tuple<T...>& tuple, std::array<unsigned char, N> const& src)
{
    return detail::deserialize(tuple, src.data(), src.data() + src.size(), std::make_index_sequence<sizeof...(T)>{});
}

template<class... T> auto deserialize(std::tuple<T...>& tuple, emb::contiguous_buffer<unsigned char>&& src)
{
    return detail::deserialize(tuple, src.data(), src.data() + src.size(), std::make_index_sequence<sizeof...(T)>{});
}

}

#endif // EMB_SER_SERIALIZE_HPP