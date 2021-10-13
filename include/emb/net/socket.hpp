#ifndef EMB_NET_SOCKET_HPP
#define EMB_NET_SOCKET_HPP

#include <emb/contiguous_buffer.hpp>
#include <array>
#include <vector>
#include <tuple>
#include <string_view>
#include <type_traits>

namespace emb::net
{

class socket
{
public:
    socket(int domain, int type, int protocol);
    explicit socket(int fd);
    socket();
    socket(socket&& rhs) noexcept;
    socket& operator=(socket const&) = delete;
    socket& operator=(socket&& rhs) noexcept;
    virtual ~socket();
    void close() noexcept;
    int descriptor() const noexcept;
    bool is_null() const noexcept;
    socket accept() const noexcept;

    // obsolete
    std::vector<unsigned char> read(std::size_t count) const;

    template<std::size_t N>
    auto read_some() const {
        std::tuple<std::size_t, std::array<unsigned char, N>> res;
        std::get<0>(res) = read_some(std::get<1>(res).data(), N);
        return res;
    }

    template<std::size_t N>
    auto read() const {
        std::array<unsigned char, N> res;
        read(res.data(), N);
        return res;
    }

    template<class T>
    auto read() const {
        static_assert(std::is_trivially_copyable_v<T>);
        T result;
        read(reinterpret_cast<unsigned char*>(&result), sizeof(T));
        return result;
    }

    void read(emb::contiguous_buffer<unsigned char> dest, std::size_t count);
    
    void write(std::vector<char> data) const;
    void write(std::string_view data) const;
    void write(char unsigned const* data, std::size_t size) const;

    template<class T>
    void write(T const& data) const {
        static_assert(std::is_trivially_copyable_v<T>);
        write(reinterpret_cast<char unsigned const*>(&data), sizeof(T));
    }

    template<class T, std::size_t N>
    void write(T const (& data)[N]) const {
        static_assert(std::is_trivially_copyable_v<T>);
        write(reinterpret_cast<char unsigned const*>(&data[0]), N*sizeof(T));
    }

private:
    int read_some(unsigned char* data, std::size_t size) const;
    void read(unsigned char* data, std::size_t size) const;

private:
    int descriptor_;
};

} // namespace emb::net

#endif // EMB_NET_SOCKET_HPP