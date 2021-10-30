#ifndef EMB_NET_EXCEPTIONS_HPP
#define EMB_NET_EXCEPTIONS_HPP

#include <stdexcept>

namespace emb::net
{

class connection_closed : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

}

#endif // EMB_NET_EXCEPTIONS_HPP