#include "emb/call_chain.hpp"
#include "emb/net/transfer_chain.hpp"
#include "emb/net/async_socket_server.hpp"
#include "emb/net/uds_client_socket.hpp"
#include "emb/net/uds_server_socket.hpp"
#include <thread>
#include "catch.hpp"

TEST_CASE("Test server-side chain", "[transfer_chain]")
{
    constexpr auto path = "/tmp/test.socket";
    char message[] = "Hello world";
    emb::net::uds_server_socket uss{path};
    emb::net::async_socket_server ass{uss};
    bool run = true;

    namespace tc = emb::net::transfer_chain;
    std::array<unsigned char, 128> buffer;
    auto chain = tc::root(ass, 0, buffer) 
            // read sizeof(size_t) to the buffer
            >> tc::read(sizeof(std::size_t))
            // then get the data from the buffer, interpet is as the next chunk size and execute the next read
            >> tc::read([](auto buffer) { std::size_t *ptr = (std::size_t *)buffer.data(); return *ptr; }) // sry for UB and C-cast
            // CHECK if proper message was received
            >> tc::process([&](auto buffer) { CHECK(std::memcmp(message, buffer.data(), sizeof(message)) == 0); run = false; })
        ;

    // activate a chain on accept
    ass.on_accept = [&](auto id) { chain.run(id); };

    // run the server!
    std::thread thr {
        [&] {
            while (run) {
                ass.run_once();
            }
        }
    };

    emb::net::uds_client_socket ucs{path};

    std::array<unsigned char, 128> out_buffer;
    std::size_t size = sizeof(message);

    std::memcpy(out_buffer.data(), &size, sizeof(size));
    ucs.write_some(emb::contiguous_buffer{out_buffer, sizeof(size)});

    std::memcpy(out_buffer.data(), message, sizeof(message));
    ucs.write_some(emb::contiguous_buffer{out_buffer, sizeof(message)});

    thr.join();
}

TEST_CASE("Chain call methods", "[call_chain]")
{
    bool first_called = false;
    bool second_called = false;

    auto cc = emb::call_chain{
        [&] { first_called = true; },
        [&] { second_called = true; }
    };

    REQUIRE(first_called == false);
    REQUIRE(second_called == false);

    cc.step();

    REQUIRE(first_called == true);
    REQUIRE(second_called == false);

    cc.step();

    REQUIRE(first_called == true);
    REQUIRE(second_called == true);
}

TEST_CASE("Append a call", "[call_chain]")
{
    bool called = false;
    auto cc = emb::call_chain{
        [&] { }
    };

    auto cc2 = cc.append([&] { called = true; });

    cc2.step();
    cc2.step();
    REQUIRE(called);
}

TEST_CASE("Move append on temporary", "[call_chain]")
{
    struct move_only {
        move_only(move_only const&) = delete;
        move_only() = default;
        move_only(move_only&&) = default;

        void operator()() const {}
    };

    // auto cc = emb::call_chain{move_only{}};
    // cc.append(move_only{}); - compiler error

    // calling append on a temporary is correct
    auto cc = emb::call_chain{move_only{}}.append(move_only{});
}