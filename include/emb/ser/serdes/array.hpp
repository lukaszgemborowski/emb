#ifndef EMB_SER_SERDES_ARRAY_HPP
#define EMB_SER_SERDES_ARRAY_HPP

#include <emb/ser/serdes/serdes.hpp>
#include <array>

namespace emb::ser::detail
{

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

} // namespace emb::ser::detail

#endif // EMB_SER_SERDES_ARRAY_HPP