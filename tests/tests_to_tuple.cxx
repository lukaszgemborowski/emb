#include "emb/tup/to_tuple.hpp"
#include "catch.hpp"

TEST_CASE("Convert struct to tuple", "[tup][to_tuple]")
{
    struct F {
        int a;
        char c;
    };

    auto Ftuple = emb::tup::to_tuple(F{42, 'x'});

    CHECK(std::is_same_v<decltype(Ftuple), std::tuple<int, char>>);
    CHECK(std::get<0>(Ftuple) == 42);
    CHECK(std::get<1>(Ftuple) == 'x');
}