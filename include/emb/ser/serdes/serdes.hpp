#ifndef EMB_SER_SERDES_SERDES_HPP
#define EMB_SER_SERDES_SERDES_HPP

#include <cstring>
#include <type_traits>

namespace emb::ser::detail
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

} // namespace emb::ser::detail

#endif // EMB_SER_SERDES_SERDES_HPP