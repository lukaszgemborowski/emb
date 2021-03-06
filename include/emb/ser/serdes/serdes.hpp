#ifndef EMB_SER_SERDES_SERDES_HPP
#define EMB_SER_SERDES_SERDES_HPP

#include <cstring>
#include <type_traits>

namespace emb::ser::detail
{

/**
 * \brief Base template for serialize/deserialize.
 */
template<class T>
struct serdes {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

    static std::size_t serialize(T const& object, unsigned char* curr, unsigned char* end) {
        if (end - curr < sizeof(T)) {
            return -1;
        }

        std::memcpy(curr, &object, sizeof(object));
        return sizeof(T);
    }

    static std::size_t deserialize(T& object, unsigned char const* curr, unsigned char const* end) {
        if (end - curr < sizeof(T)) {
            return -1;
        }

        std::memcpy(&object, curr, sizeof(T));
        return sizeof(T);
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

} // namespace emb::ser::detail

#endif // EMB_SER_SERDES_SERDES_HPP