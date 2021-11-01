#include "emb/net/socket.hpp"
#include <sys/socket.h>
#include <stdexcept>
#include <unistd.h>

namespace emb::net
{

socket::socket(int domain, int type, int protocol)
    : descriptor_ {::socket(domain, type, protocol)}
{
    if (descriptor_ == -1) {
        throw std::runtime_error{"socket() failed"};
    }
}

socket::socket(int fd)
    : descriptor_ {fd}
{}

socket::socket()
    : descriptor_ {-1}
{}

socket::socket(socket&& rhs) noexcept
{
    close();
    descriptor_ = rhs.descriptor_;
    rhs.descriptor_ = -1;
}

socket& socket::operator=(socket&& rhs) noexcept {
    close();
    descriptor_ = rhs.descriptor_;
    rhs.descriptor_ = -1;
    return *this;
}

socket::~socket() {
    if (descriptor_ != -1) {
        ::close(descriptor_);
    }
}

int socket::descriptor() const noexcept {
    return descriptor_;
}

bool socket::is_null() const noexcept {
    return descriptor_ == -1;
}

socket socket::accept() const noexcept {
    return socket{::accept(descriptor_, nullptr, nullptr)};
}

void socket::close() noexcept {
    if (descriptor_ != -1) {
        ::close(descriptor_);
        descriptor_ = -1;
    }
}

int socket::read_some(unsigned char* data, std::size_t size) const
{
    return ::read(descriptor(), data, size);
}

void socket::read(unsigned char* data, std::size_t size) const
{
    std::size_t total = 0;

    do {
        auto r = ::read(descriptor(), data + total, size - total);

        if (r < 0) {
            throw std::runtime_error("read() failed");
        } else if (r == 0) {
            throw connection_closed{"read"};
        }

        total += r;
    }
    while (total < size);
}

int socket::write_some(char unsigned const* data, std::size_t size) const {
    auto r = ::write(descriptor(), data, size);

    if (r == -1) {
        throw std::runtime_error{"write() failed"};
    } else {
        return r;
    }
}

} // namespace emb::net