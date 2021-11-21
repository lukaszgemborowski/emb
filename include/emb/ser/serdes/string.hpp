#ifndef EMB_SER_SERDES_STRING_HPP
#define EMB_SER_SERDES_STRING_HPP

#include <emb/ser/serdes/serdes.hpp>
#include <string>

namespace emb::ser::detail
{

template<>
struct serdes<std::string>
{
    using size_type = std::uint16_t;
    using size_type_serializer = serdes<size_type>;

    static_assert(size_type_serializer::is_constant_size());

    static std::size_t serialize(std::string const& object, unsigned char* curr, unsigned char* end) {
        if (object.size() > std::numeric_limits<size_type>::max()) {
            return -1;
        }

        if (end - curr < object.size() + size_type_serializer::min_size()) {
            return -1;
        }

        size_type size = static_cast<size_type>(object.size());
        auto read_size = size_type_serializer::serialize(size, curr, end);

        if (read_size == -1) {
            return -1;
        }

        std::memcpy(curr + size_type_serializer::min_size(), object.c_str(), size);
        return read_size + size;
    }

    static std::size_t deserialize(std::string& object, unsigned char const* curr, unsigned char const* end) {
        if (end - curr < size_type_serializer::min_size()) {
            return false;
        }

        size_type size = 0;
        auto read_size = size_type_serializer::deserialize(size, curr, end);

        if (read_size == -1) {
            return -1;
        }

        if (end - curr < size) {
            return -1;
        }

        object = std::string{reinterpret_cast<const char*>(curr + size_type_serializer::min_size()), size};
        return read_size + size;
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

} // namespace emb::ser::detail

#endif // EMB_SER_SERDES_STRING_HPP