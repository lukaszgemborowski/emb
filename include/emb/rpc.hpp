#pragma once
#include <type_traits>
#include <tuple>
#include <map>
#include "emb/serialization.hpp"
#include "emb/socket.hpp"

namespace emb::rpc
{

template<class T, class... U>
struct prepend_tuple ;

template<class... T, class... U>
struct prepend_tuple<std::tuple<T...>, U...> {
    using type = std::tuple<U..., T...>;
};

template<auto Id, typename Signature>
struct procedure {
    template<auto ToCheck>
    static constexpr bool compare_id() {
        if constexpr (std::is_same_v<decltype(Id), decltype(ToCheck)>) {
            return Id == ToCheck;
        } else {
            return false;
        }
    }

    template<class R, class... Args>
    struct types {
        using return_type = R;
        using arguments_tuple = std::tuple<Args...>;
        using std_function = std::function<R (Args...)>;
    };

    template<class R, class... Args>
    static constexpr types<R, Args...> get_types_impl(R (*)(Args...)) {
        return {};
    }

    static constexpr auto get_types() {
        return get_types_impl(static_cast<Signature *>(nullptr));
    }

    static constexpr auto get_id() {
        return Id;
    }
};

template<typename... Functions>
struct api {
    template<auto Id>
    constexpr auto get_signature() const {
        return get_sig_impl<Id, Functions...>();
    }

    struct none {};

    template<auto Id, class F, class... T>
    constexpr auto get_sig_impl() const {
        if constexpr (F::template compare_id<Id>()) {
            return F{};
        } else {
            if constexpr (sizeof... (T) == 0) {
                return none {};
            } else {
                return get_sig_impl<Id, T...>();
            }
        }
    }

    template<class F>
    constexpr auto transform() const {
        return transform_impl<F, Functions...>();
    }

    template<class F, class... U>
    constexpr auto transform_impl() const {
        return transformation<typename F::template type<U>...>{};
    }

    template<class... T>
    struct transformation {
        using as_tuple = std::tuple<T...>;
    };
};

template<typename... Functions>
constexpr auto make_api(Functions...) {
    return api<Functions...>{};
}

template<class Api>
class caller
{
public:
    caller(Api, emb::socket& sck)
        : socket_ {sck}
    {}

    template<auto Id, class... Args>
    void send(Args... args) {
        using types = decltype(Api{}.template get_signature<Id>().get_types());
        typename prepend_tuple<typename types::arguments_tuple, int>::type tuple{
            static_cast<int>(Id), args...};

        if (auto size = ser::serialize(tuple, &buffer_[0], &buffer_[0] + buffer_.size()); size > 0) {
            socket_.write(&buffer_[0], size);
        }
    }

private:
    emb::socket& socket_;
    std::array<unsigned char, 512> buffer_;
};


namespace detail
{

template<auto Id, class Callback>
struct callback {
    static constexpr auto id = Id;
    Callback callback;
};

}

template<class Api>
class server {
public:
    server(Api, emb::socket& sck)
        : multiplexer_ {sck, {*this, &server<Api>::accepted, &server<Api>::ready}}
    {}

    template<auto Id, class Func>
    void set_callback(Func func) {
        set_callback_impl<Id>(func, std::make_index_sequence<std::tuple_size_v<callbacks_tuple>>{});
    }

    template<auto Id, class Func, std::size_t... I>
    void set_callback_impl(Func func, std::index_sequence<I...>) {
        auto set_if_id = [&](auto& cb) {
            if constexpr (cb.id == Id) {
                cb.callback = func;
            }
        };

        (set_if_id(std::get<I>(callbacks_)), ...);
    }

    void run()
    {
        multiplexer_.run();
    }

private:
    void accepted(emb::socket&) {}
    void ready(int id, emb::socket& sck) {
        auto &buffer = buffers_[id];
    }

    struct transform {
        template<class T>
        struct type_impl {
            using procedure_types = decltype(T::get_types());
            using result = detail::callback<
                T::get_id(),
                typename procedure_types::std_function>;
        };

        template<class T>
        using type = typename type_impl<T>::result;
    };

    using callbacks_tuple = typename decltype(Api{}.template transform<transform>())::as_tuple;

private:
    emb::multiplexed_server<emb::method_dispatcher<server<Api>>>
        multiplexer_;
    callbacks_tuple callbacks_;
    // TODO: make it static
    std::map<int, std::vector<char>> buffers_;
};


}
