#ifndef EMB_SPAN_HPP
#define EMB_SPAN_HPP

#include <type_traits>
#include <vector>
#include <limits>
#include <array>

namespace emb
{

constexpr std::size_t npos = -1;

template<class T>
class contiguous_buffer
{
public:
    using constless_type  = std::remove_const_t<T>;
    using size_type = std::size_t;
    using element_type = T;
    using pointer = T*;
    using reference = T&;

    contiguous_buffer() = default;
    contiguous_buffer(pointer begin, pointer end)
        : begin_ {begin}
        , end_ {end}
    {}

    contiguous_buffer(pointer begin, size_type size)
        : begin_ {begin}
        , end_ {begin + size}
    {
    }

    contiguous_buffer(contiguous_buffer<constless_type> const& other)
        : begin_ {other.data()}
        , end_ {other.data() + other.size()}
    {}

    contiguous_buffer(contiguous_buffer<const constless_type> const& other)
        : begin_ {other.data()}
        , end_ {other.data() + other.size()}
    {}

    template<std::size_t N>
    contiguous_buffer(std::array<T, N>& src)
        : begin_ {src.data()}
        , end_ {src.data() + src.size()}
    {}

    template<std::size_t N>
    contiguous_buffer(std::array<T, N>& src, std::size_t count)
        : begin_ {src.data()}
        , end_ {count <= N ? src.data() + count : src.data() + N}
    {}

    contiguous_buffer(std::vector<T>& src)
        : begin_ {src.data()}
        , end_ {src.data() + src.size()}
    {}

    pointer data() {
        return begin_;
    }

    const pointer data() const {
        return begin_;
    }

    const size_type size() const {
        return end_ - begin_;
    }

    const size_type bytes_count() const {
        return reinterpret_cast<const char *>(end_) - reinterpret_cast<const char *>(begin_);
    }

    contiguous_buffer<T> slice(std::size_t offset, std::size_t count) const {
        return {begin_ + offset, count == npos ? size() - offset : count};
    }

private:
    pointer    begin_ = nullptr;
    pointer    end_ = nullptr;
};

}

#endif // EMB_SPAN_HPP