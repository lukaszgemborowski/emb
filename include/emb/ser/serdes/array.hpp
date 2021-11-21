#ifndef EMB_SER_SERDES_ARRAY_HPP
#define EMB_SER_SERDES_ARRAY_HPP

#include <emb/ser/serdes/serdes.hpp>
#include <array>

namespace emb::ser::detail
{

template<class T, std::size_t N>
struct serdes<std::array<T, N>> {
    static std::size_t serialize(std::array<T, N> const& object, unsigned char* curr, unsigned char* end) {
        if (end - curr  < min_size()) {
            return -1;
        }

        std::size_t result = 0;
        for (std::size_t i = 0; i < N; ++i) {
            if (auto size = serdes<T>::serialize(object[i], curr + result, end); size >= 0) {
                result += size;
            } else {
                return -1;
            }
        }

        return result;
    }

    static std::size_t deserialize(std::array<T, N>& object, unsigned char const* curr, unsigned char const* end) {
        if (end - curr < min_size()) {
            return false;
        }

        std::size_t result = 0;
        for (std::size_t i = 0; i < N; ++i) {
            if (auto size = serdes<T>::deserialize(object[i], curr + result, end); size >= 0) {
                result += size;
            } else {
                return -1;
            }
        }

        return result;
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