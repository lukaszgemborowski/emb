#ifndef EMB_FLAGS_HPP
#define EMB_FLAGS_HPP

#include <type_traits>

namespace emb
{

namespace detail
{
template<class T>
constexpr auto is_power_of_2(T in) {
    auto n = static_cast<std::underlying_type_t<T>>(in);
    return (n & (n - 1)) == 0;
}
}

template<class E>
class flags {
private:
    using raw_type = std::underlying_type_t<E>;
public:
    using flag = E;

    constexpr flags() = default;

    template<class... V>
    constexpr flags(V... value) {
        raw_ = (static_cast<raw_type>(value) | ...);
    }

    template<flag F>
    constexpr bool is() const {
        return raw_ & static_cast<raw_type>(F);
    }

    constexpr bool is(flag f) const {
        return raw_ & static_cast<raw_type>(f);
    }

    template<flag F>
    constexpr void set() {
        raw_ |= static_cast<raw_type>(F);
    }

    template<flag F>
    constexpr void clear() {
        raw_ = raw_ & ~static_cast<raw_type>(F);
    }

    static constexpr raw_type as_integral(flag f) {
        return static_cast<raw_type>(f);
    }

private:
    raw_type raw_ = 0;
};

}

#endif // EMB_FLAGS_HPP