#ifndef EMB_SPAN_HPP
#define EMB_SPAN_HPP

#include <type_traits>
#include <limits>
#include <array>

namespace emb
{

template<class T>
class contiguous_buffer
{
public:
    using size_type = std::size_t;
    using element_type = T;
    using pointer = T*;
    using reference = T&;

    contiguous_buffer(pointer begin, pointer end)
        : begin_ {begin}
        , end_ {end}
    {}

    contiguous_buffer(pointer begin, size_type size)
        : begin_ {begin}
        , end_ {begin + size}
    {}

    template<std::size_t N>
    contiguous_buffer(std::array<T, N>& src)
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

private:
    pointer    begin_;
    pointer    end_;
};

}

#endif // EMB_SPAN_HPP