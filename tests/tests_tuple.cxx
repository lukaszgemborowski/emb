#include "emb/cpp/tuple.hpp"
#include "catch.hpp"

namespace
{
constexpr auto t1 = std::tuple<char, int, short, bool>{};
constexpr auto t2 = emb::cpp::tuple_slice<0, 4>(t1);

static_assert(std::is_same_v<
    decltype(t2),
    decltype(t1)
>);

constexpr auto t3 = emb::cpp::tuple_slice<0, 2>(t1);

static_assert(std::is_same_v<
    decltype(t3),
    const std::tuple<char, int>
>);

constexpr auto t4 = emb::cpp::tuple_slice<2, 2>(t1);

static_assert(std::is_same_v<
    decltype(t4),
    const std::tuple<short, bool>
>);

}
