#include "emb/cpp/tuple.hpp"
#include "catch.hpp"

namespace
{
constexpr auto t1 = std::tuple<char, int, short, bool>{'c', 42, 2, true};
}

TEST_CASE("Slice whole tuple", "[cpp][tuple_slice]")
{
    constexpr auto t2 = emb::cpp::tuple_slice<0, 4>(t1);

    static_assert(std::is_same_v<decltype(t2), decltype(t1)>);
    REQUIRE(t1 == t2);
}

TEST_CASE("Slice part of the tuple", "[cpp][tuple_slice]")
{
    SECTION("First two elements")
    {
        constexpr auto t2 = emb::cpp::tuple_slice<0, 2>(t1);

        static_assert(std::is_same_v<decltype(t2), const std::tuple<char, int>>);
        REQUIRE(t2 == std::tuple<char, int>{'c', 42});
    }

    SECTION("Last two elements")
    {
        constexpr auto t2 = emb::cpp::tuple_slice<2, 2>(t1);

        static_assert(std::is_same_v<decltype(t2), const std::tuple<short, bool>>);
        REQUIRE(t2 == std::tuple<short, bool>{2, true});
    }
}

// TEST_CASE("Slice 0-sized tuple", "[cpp][tuple_slice]")
//{
    constexpr auto t2 = emb::cpp::tuple_slice<0, 0>(t1);
    static_assert(std::is_same_v<decltype(t2), const std::tuple<>>);
//}