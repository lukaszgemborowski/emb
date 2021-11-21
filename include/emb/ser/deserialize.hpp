#ifndef EMB_SER_DESERIALIZE_HPP
#define EMB_SER_DESERIALIZE_HPP

#include <emb/contiguous_buffer.hpp>
#include <emb/ser/serdes/tuple.hpp>
#include <emb/ser/serdes/array.hpp>
#include <emb/ser/serdes/string.hpp>

namespace emb::ser
{
namespace detail
{

template<class T>
std::size_t deserialize(T& tuple, unsigned char const* begin, unsigned char const* end)
{
    return serdes<T>::deserialize(tuple, begin, end);
}

} // namespace detail

inline auto deserialize_size(emb::contiguous_buffer<const unsigned char> buffer)
{
    if (buffer.size() < sizeof(std::uint32_t)) {
        return invalid_size;
    }

    tuple_size_type size = 0;

    if (detail::serdes<tuple_size_type>::deserialize(size, buffer.data(), buffer.data() + buffer.size())) {
        return size + static_cast<tuple_size_type>(sizeof(tuple_size_type));
    } else {
        return invalid_size;
    }
}

/**
 * \brief Deserialize raw buffer into the tuple.
 * \param tuple output tuple
 * \param begin beginning of the source buffer
 * \param end   past the end of the source buffer
 * \return Bytes read from the buffer or -1 on error.
 */
template<class... T> auto deserialize(std::tuple<T...>& tuple, unsigned char* begin, unsigned char* end)
{
    return detail::deserialize(tuple, begin, end);
}

/**
 * \brief Deserialize std::array into the tuple.
 * \param tuple output tuple
 * \param src   source buffer
 * \return Bytes read from the buffer or -1 on error.
 */
template<std::size_t N, class... T> auto deserialize(std::tuple<T...>& tuple, std::array<unsigned char, N> const& src)
{
    return detail::deserialize(tuple, src.data(), src.data() + src.size());
}

/**
 * \brief Deserialize emb::contiguous_buffer into the tuple.
 * \param tuple output tuple
 * \param src   source buffer
 * \return Bytes read from the buffer or -1 on error.
 */
template<class... T> auto deserialize(std::tuple<T...>& tuple, emb::contiguous_buffer<unsigned char>&& src)
{
    return detail::deserialize(tuple, src.data(), src.data() + src.size());
}

}

#endif // EMB_SER_DESERIALIZE_HPP