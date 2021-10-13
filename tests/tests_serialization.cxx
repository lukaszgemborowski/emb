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

    CHECK(std::get<0>(todeser) == std::get<0>(toser));
    CHECK(std::get<1>(todeser) == std::get<1>(toser));
    CHECK(std::get<2>(todeser) == std::get<2>(toser));
}