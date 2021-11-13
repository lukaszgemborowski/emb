#include "catch.hpp"
#include <emb/ser/serialize.hpp>
#include <emb/ser/deserialize.hpp>

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
    CHECK(sizes.min == 17);

    CHECK(std::get<0>(todeser) == std::get<0>(toser));
    CHECK(std::get<1>(todeser) == std::get<1>(toser));
    CHECK(std::get<2>(todeser) == std::get<2>(toser));
}

TEST_CASE("Serialize an std::array", "[serialize]")
{
    std::tuple<int, std::array<char, 6>> toser{42, {'h', 'e', 'l', 'l', 'o', 0}};
    decltype(toser) todeser;
    std::array<unsigned char, 64> buffer;

    REQUIRE(emb::ser::serialize(toser, emb::contiguous_buffer{buffer}) > 0);
    REQUIRE(emb::ser::deserialize(todeser, emb::contiguous_buffer{buffer}) > 0);

    REQUIRE(std::get<0>(toser) == std::get<0>(todeser));
    REQUIRE(std::get<1>(toser) == std::get<1>(todeser));
}

TEST_CASE("Serialize a size of the tuple", "[serialize]")
{
    using tuple_t = std::tuple<char, char, char>;
    static constexpr auto three_char_size = sizeof(char) * 3;
    tuple_t t;
    std::array<unsigned char, 64> buffer;
    unsigned char* beg = buffer.data();
    unsigned char* end = buffer.data() + buffer.size();

    auto serialized_size = emb::ser::detail::serdes<tuple_t>::serialize(t, beg, end);
    REQUIRE(serialized_size > three_char_size);

    std::uint32_t written_size = 0;
    const unsigned char* beg2 = buffer.data();
    emb::ser::detail::serdes<std::uint32_t>::deserialize(written_size, beg2, end);

    CHECK(written_size == three_char_size);
    CHECK(three_char_size + sizeof(emb::ser::tuple_size_type) == emb::ser::deserialize_size(emb::contiguous_buffer{buffer}));
    CHECK(emb::ser::deserialize_size(emb::contiguous_buffer{buffer}) == emb::ser::size_requirements(t).min);
}

TEST_CASE("Serialize a std::string", "[serialize]")
{
    std::tuple<std::string> t1{"foo"};
    std::tuple<std::string> t2;
    std::array<unsigned char, 64> buff;
    auto size = emb::ser::serialize(t1, buff);
    CHECK(size == 9);
    CHECK(emb::ser::deserialize(t2, buff) == size);
    CHECK(t1 == t2);
}