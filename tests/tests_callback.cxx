#include "emb/callback.hpp"
#include "catch.hpp"

TEST_CASE("Create and call", "[callback]")
{
    bool called = false;
    emb::callback<void ()> cb;
    cb.set([&] { called = true; });
    REQUIRE(called == false);
    REQUIRE(cb.is_set());
    cb.call();
    REQUIRE(called);
}

TEST_CASE("Create via public accessor", "[callback]")
{
    emb::callback<void ()> cb;
    emb::public_callback<void ()> pcb{cb};

    REQUIRE(cb.is_set() == false);
    pcb = [] {};
    REQUIRE(cb.is_set());
}

namespace
{
class CallbackUseCase
{
private:
    emb::callback<void ()> cb_;

public:
    emb::public_callback<void ()> a_callback {cb_};
};
}

TEST_CASE("Typical callback use case", "[callback]")
{
    CallbackUseCase a_class;

    a_class.a_callback = [] {};
}