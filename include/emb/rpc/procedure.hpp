#ifndef EMB_RPC_PROCEDURE_HPP
#define EMB_RPC_PROCEDURE_HPP

#include <type_traits>
#include <tuple>
#include <functional>

namespace emb::rpc
{

template<auto Id, typename Signature>
struct procedure {
    using id_type = decltype(Id);

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

}

#endif // EMB_RPC_PROCEDURE_HPP