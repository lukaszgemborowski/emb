#include "emb/string.hpp"
#include "catch.hpp"

TEST_CASE("Creating a string of 6 characters", "[string]")
{
    auto foo = "foobar";
    
    SECTION("size 10")
    {
        emb::string<10> str{foo};
        REQUIRE(std::strlen(foo) == str.size());
    }

    SECTION("size 7")
    {
        emb::string<7> str{foo};
        REQUIRE(6 == str.size());
    }

    SECTION("size 6")
    {
        emb::string<6> str{foo};
        REQUIRE(5 == str.size());
    }

    SECTION("Enough storage for limited string")
    {
        emb::string<10> str{foo, 5};
        REQUIRE(str.size() == 5);
    }
}