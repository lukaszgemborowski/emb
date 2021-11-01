#ifndef EMB_RPC_CALLBACK_TUPLE_HPP
#define EMB_RPC_CALLBACK_TUPLE_HPP

#include <type_traits>
#include <tuple>

namespace emb::rpc
{

namespace detail
{

template<auto Id, class ProcTypes>
struct callback {
    static constexpr auto id = Id;
    typename ProcTypes::std_function callback;
    using arguments_tuple = typename ProcTypes::arguments_tuple;
    using return_type = typename ProcTypes::return_type;
};

}

template<class Api>
class callback_tuple
{
private:
    struct transform {
        template<class T>
        struct type_impl {
            using procedure_types = decltype(T::get_types());
            using result = detail::callback<
                T::get_id(),
                procedure_types>;
        };

        template<class T>
        using type = typename type_impl<T>::result;
    };

    using callbacks_tuple = typename decltype(Api{}.template transform<transform>())::as_tuple;

public:
    template<auto Id, class Func>
    void set_callback(Func func) {
        set_callback_impl<Id>(func, std::make_index_sequence<std::tuple_size_v<callbacks_tuple>>{});
    }

    template<class Fun>
    void visit(int id, Fun visitor) {
        visit_impl(id, visitor, std::make_index_sequence<std::tuple_size_v<callbacks_tuple>>{});
    }

private:
    template<auto Id, class Func, std::size_t... I>
    void set_callback_impl(Func func, std::index_sequence<I...>) {
        auto set_if_id = [&](auto& cb) {
            if constexpr (cb.id == Id) {
                cb.callback = func;
            }
        };

        (set_if_id(std::get<I>(callbacks_)), ...);
    }

    template<class Fun, std::size_t C, std::size_t... I>
    void visit_impl(int id, Fun fun, std::index_sequence<C, I...>) {
        if (static_cast<decltype(std::get<C>(callbacks_).id)>(id) == std::get<C>(callbacks_).id) {
            fun(std::get<C>(callbacks_));
        }

        visit_impl(id, fun, std::index_sequence<I...>{});
    }

    template<class Fun>
    void visit_impl(int, Fun, std::index_sequence<>) { }

private:
    callbacks_tuple callbacks_;
};

}

#endif // EMB_RPC_CALLBACK_TUPLE_HPP