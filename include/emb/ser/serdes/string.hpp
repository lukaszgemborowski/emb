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

} // namespace emb::ser::detail

#endif // EMB_SER_SERDES_STRING_HPP