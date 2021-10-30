#ifndef EMB_NET_SOCKET_HPP
#define EMB_NET_SOCKET_HPP

#include <emb/contiguous_buffer.hpp>
#include <emb/net/exceptions.hpp>
#include <array>
#include <vector>
#include <tuple>
#include <string_view>
#include <type_traits>
#include <cstring>

namespace emb::net
{

class socket
{
public:
    /**
     * \brief Create a socket.
     * This is one-to-one wrapper around socket interface, the arguments
     * are exactly the same as in socket() function call, no validation
     * is performed.
     **/
    socket(int domain, int type, int protocol);

    /**
     * \brief Create a socket object from existing file descriptor.
     **/
    explicit socket(int fd);

    /**
     * \brief Create an empty/invalid socket object.
     * It may be used to move-in another socket object.
     **/
    socket();

    socket& operator=(socket&& rhs) noexcept;
    socket(socket&& rhs) noexcept;

    socket& operator=(socket const&) = delete;
    socket(socket const&) = delete;

    virtual ~socket();

    /**
     * \brief Close the socket.
     * The socket will be invalid from this point.
     **/ 
    void close() noexcept;

    /**
     * \brief Get raw file descriptor.
     **/
    int descriptor() const noexcept;
    bool is_null() const noexcept;
    socket accept() const noexcept;

    /**
     * \brief Read up to N bytes.
     * This function won't block and wait for all N bytes, as soon
     * as data is available the read_some() function will return.
     **/
    template<std::size_t N>
    auto read_some() const {
        std::tuple<std::size_t, std::array<unsigned char, N>> res;
        std::get<0>(res) = read_some(std::get<1>(res).data(), N);
        return res;
    }

    auto read_some(contiguous_buffer<unsigned char> dest) const {
        return read_some(dest.data(), dest.size());
    }

    /**
     * \brief Read exactly N bytes from the socket.
     * If there is not enough data the read() will block and wait.
     **/
    template<std::size_t N>
    auto read() const {
        std::array<unsigned char, N> res;
        read(res.data(), N);
        return res;
    }

    void read(emb::contiguous_buffer<unsigned char> dest)
    {
        read(dest.data(), dest.size());
    }

    void read(emb::contiguous_buffer<char> dest)
    {
        read(reinterpret_cast<unsigned char *>(dest.data()), dest.size());
    }

    auto write_some(emb::contiguous_buffer<const unsigned char> src)
    {
        return write_some(src.data(), src.bytes_count());
    }

    auto write_some(emb::contiguous_buffer<const char> src)
    {
        return write_some(reinterpret_cast<const unsigned char *>(src.data()), src.bytes_count());
    }

private:
    int read_some(unsigned char* data, std::size_t size) const;
    void read(unsigned char* data, std::size_t size) const;
    int write_some(char unsigned const* data, std::size_t size) const;

private:
    int descriptor_;
};

} // namespace emb::net

#endif // EMB_NET_SOCKET_HPP