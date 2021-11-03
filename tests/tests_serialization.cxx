#include "catch.hpp"
#include <emb/ser/serialize.hpp>

TEST_CASE("Serialize tuples", "[serialize]")
{
    std::tuple<int, double, char> toser{42, 3.14, 'x'};
    std::tuple<int, double, char> todeser{};

    SECTION("Use pointers")
    {
        std::array<unsigned char, 64> buffer;
        emb::ser::serialize(toser, buffer.data(), buffer.data() + buffer.size());
        emb::ser::deserialize(todeser, buffer.data(), buffer.data() + buffer.size());
    }

    SECTION("Use std::array")
    {
        std::array<unsigned char, 64> buffer;
        emb::ser::serialize(toser, buffer);
        emb::ser::deserialize(todeser, buffer);
    }

    SECTION("Use contiguous_buffer")
    {
        std::array<unsigned char, 64> buffer;
        emb::ser::serialize(toser, emb::contiguous_buffer{buffer});
        emb::ser::deserialize(todeser, emb::contiguous_buffer{buffer});
    }

    auto sizes = emb::ser::size_requirements(toser);

    CHECK(sizes.min == sizes.max);
    CHECK(sizes.min == 13);

    CHECK(std::get<0>(todeser) == std::get<0>(toser));
    CHECK(std::get<1>(todeser) == std::get<1>(toser));
    CHECK(std::get<2>(todeser) == std::get<2>(toser));
}

TEST_CASE("Serialize an std::array", "[serialize]")
{
    std::tuple<int, std::array<char, 6>> toser{42, {'h', 'e', 'l', 'l', 'o', 0}};
    decltype(toser) todeser;
    std::array<unsigned char, 64> buffer;

    emb::ser::serialize(toser, emb::contiguous_buffer{buffer});
    emb::ser::deserialize(todeser, emb::contiguous_buffer{buffer});

    REQUIRE(std::get<0>(toser) == std::get<0>(todeser));
    REQUIRE(std::get<1>(toser) == std::get<1>(todeser));
}