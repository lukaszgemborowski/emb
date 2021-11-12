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

} // namespace detail

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

/**
 * \brief Serialize tuple into raw buffer.
 * \param tuple input tuple
 * \param begin beginning of the output buffer
 * \param end   past the end buffer pointer
 * \return Bytes written to the buffer or -1 on error.
 */
template<class... T> auto serialize(std::tuple<T...> const& tuple, unsigned char* begin, unsigned char* end)
{
    return detail::serialize(tuple, begin, end);
}

/**
 * \brief Serialize tuple into std::array.
 * \param tuple input tuple
 * \param dest  destination buffer
 * \return Bytes written to the buffer or -1 on error.
 */
template<std::size_t N, class... T> auto serialize(std::tuple<T...> const& tuple, std::array<unsigned char, N>& dest)
{
    return detail::serialize(tuple, dest.data(), dest.data() + dest.size());
}

/**
 * \brief Serialize tuple into emb::contiguous_buffer
 * \param tuple input tuple
 * \param dest  destination buffer
 * \return Bytes written to the buffer or -1 on error.
 */
template<class... T> auto serialize(std::tuple<T...> const& tuple, emb::contiguous_buffer<unsigned char>&& dest)
{
    return detail::serialize(tuple, dest.data(), dest.data() + dest.size());
}

}

#endif // EMB_SER_SERIALIZE_HPP