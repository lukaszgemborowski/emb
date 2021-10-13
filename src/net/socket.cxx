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

std::vector<unsigned char> socket::read(std::size_t count) const {
    std::vector<unsigned char> result;
    result.resize(count);
    auto res = ::read(descriptor(), result.data(), count);

    if (res == -1) {
        throw std::runtime_error("read() failed");
    }

    result.resize(res);
    return result;
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
            std::runtime_error("read() failed");
        }

        total += r;
    }
    while (total < size);
}

void socket::read(emb::contiguous_buffer<unsigned char> dest, std::size_t count)
{
    read(dest.data(), count);
}

void socket::write(std::vector<char> data) const {
    if (::write(descriptor(), data.data(), data.size()) == -1) {
        throw std::runtime_error{"write() failed"};
    }
}

void socket::write(std::string_view data) const {
    if (::write(descriptor(), data.data(), data.size()) == -1) {
        throw std::runtime_error{"write() failed"};
    }
}

void socket::write(char unsigned const* data, std::size_t size) const {
    if (::write(descriptor(), data, size) == -1) {
        throw std::runtime_error{"write() failed"};
    }
}

} // namespace emb::net