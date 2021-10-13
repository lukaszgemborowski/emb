#ifndef EMB_STRING_HPP
#define EMB_STRING_HPP

#include <string_view>
#include <cstring>

namespace emb
{

template<std::size_t N>
class string
{
public:
    explicit string(char const* src)
    {
        std::strncpy(buffer_, src, N);
    }

    string(char const* src, std::size_t max)
    {
        std::strncpy(buffer_, src, max < N ? max : N);
    }

    template<std::size_t M>
    explicit string(string<M> const& src)
    {
        static_assert(M <= N, "Destination buffer to small");
        std::strncpy(buffer_, src.buffer_, M);
    }

    char const* c_str() const noexcept {
        return buffer_;
    }

    auto string_view() const noexcept {
        return std::string_view{buffer_};
    }

private:
    char buffer_[N];
};

}

#endif // EMB_STRING_HPP