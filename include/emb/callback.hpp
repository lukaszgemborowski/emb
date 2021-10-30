#ifndef EMB_CALLBACK_HPP
#define EMB_CALLBACK_HPP

#include <functional>

namespace emb
{

template<class Signature>
class callback
{
public:
    using function_type = std::function<Signature>;

    template<class... Args>
    auto call(Args&&... args) {
        return func_(std::forward<Args>(args)...);
    }

    void set(function_type f) {
        func_ = f;
    }

    bool is_set() const {
        return static_cast<bool>(func_);
    }

private:
    function_type func_;
};

template<class Signature>
class public_callback
{
public:
    public_callback(callback<Signature>& p)
        : cb_ {p}
    {}

    void operator=(typename callback<Signature>::function_type f) {
        cb_.set(f);
    }

    operator bool () const {
        return cb_.is_set();
    }

private:
    callback<Signature>& cb_;
};

}

#endif // EMB_CALLBACK_HPP