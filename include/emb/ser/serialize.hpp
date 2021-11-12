#ifndef EMB_SER_SERIALIZE_HPP
#define EMB_SER_SERIALIZE_HPP

#include <emb/contiguous_buffer.hpp>
#include <emb/ser/serdes/tuple.hpp>
#include <emb/ser/serdes/array.hpp>
#include <emb/ser/serdes/string.hpp>
#include <tuple>
#include <type_traits>
#include <array>

namespace emb::ser
{

using tuple_size_type = std::uint32_t;
constexpr tuple_size_type invalid_size = 0xffffffff;

namespace detail
{

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