#ifndef EMB_STRING_HPP
#define EMB_STRING_HPP

#include <string_view>
#include <cstring>

namespace emb
{

/**
 * \brief Statically allocated string of size N.
 * 
 * Array of size N is used as a storage for the string. The string
 * is stored as NULL terminated string. There are no dynamic allocations.
 */
template<std::size_t N>
class string
{
public:
    /**
     * \brief Construct a string from a char const* C-string.
     * \param src Input C-string.
     * 
     * The string is created by strncpy of the source C-string.
     */
    explicit string(char const* src)
    {
        std::strncpy(buffer_, src, N);
        buffer_[N-1] = 0;
    }

    /**
     * \brief Construct a string from limited-size C-string.
     * \param src Input C-string.
     * \param max Maximum number of characters to copy.
     * 
     * The string is created by strncpy of the source C-string limited
     * to the provided parameter (or N, whichver is less).
     */
    string(char const* src, std::size_t max)
    {
        auto const to_copy = max < N ? max : N;
        std::strncpy(buffer_, src, to_copy);

        if (to_copy < N) {
            buffer_[to_copy] = 0;
        } else {
            buffer_[N-1] = 0;
        }
    }

    /**
     * \brief Construct a string from another string.
     * \param src Source string object.
     * 
     * The source string must be shorter or equal than the
     * destination string. Otherwise static_assert will fail.
     */
    template<std::size_t M>
    explicit string(string<M> const& src)
    {
        static_assert(M <= N, "Destination buffer to small");
        std::strncpy(buffer_, src.buffer_, M);
    }

    /**
     * \brief Get raw c-string.
     */
    char const* c_str() const noexcept {
        return buffer_;
    }

    /**
     * \brief Create string_view from underlying string.
     */
    auto string_view() const noexcept {
        return std::string_view{buffer_};
    }

    /**
     * \brief Get size of the string.
     */
    auto size() const noexcept {
        return std::strlen(buffer_);
    }

private:
    char buffer_[N];
};

}

#endif // EMB_STRING_HPP