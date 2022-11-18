#pragma once
#include <netinet/in.h>
#include <string_view>
#include <arpa/inet.h>
#include <cstring>
#include <cerrno>
#include <stdexcept>
#include <string>
#include <iostream>
#include <iomanip>
#include <utility>

namespace sockets {

    class endpoint {
    public:
        /**
         * @param address numbers-and-dots notation address e.g. 127.0.0.1
         * @param port_number
         */
        endpoint(std::string_view address, uint16_t port_number)
        : m_address_in() {
            m_address_in.sin_family = AF_INET;
            const auto result = inet_aton(address.data(), &m_address_in.sin_addr);
            if (result == 0)
                throw std::invalid_argument("address");
            m_address_in.sin_port = htons(port_number);
        }
        /**
         *
         * @param address ip address in bytes representation
         * @param port_number
         */
        endpoint(in_addr_t address, uint16_t port_number) noexcept
        : m_address_in() {
            m_address_in.sin_family = AF_INET;
            m_address_in.sin_addr.s_addr = htonl(address);
            m_address_in.sin_port = htons(port_number);
        }

        explicit endpoint(const sockaddr_in& addr) noexcept
        :m_address_in(addr){}

        endpoint(const sockaddr& addr, size_t size) noexcept
        : m_address_in(){
            std::memcpy(&m_address_in, &addr, size);
        }

        explicit endpoint(const sockaddr_storage& storage) noexcept
        : m_address_in() {
            std::memcpy(&m_address_in, &storage, sizeof(storage));
        }

        /**
         * Creates an invalid endpoint
         */
        endpoint() noexcept = default;

        endpoint(endpoint const &ep) noexcept = default;

        endpoint(endpoint&& ep) noexcept = default;


        endpoint& operator=(const endpoint& rhs) noexcept = default;

        endpoint& operator=(endpoint&& rhs) noexcept = default;

        /**
         * @return The capacity of the underlying socket endpoint
         */
        [[nodiscard]] const sockaddr* data() const noexcept {
            return (sockaddr*)&m_address_in;
        }

        /**
         * @return The capacity of the underlying socket endpoint
         */
        [[nodiscard]] size_t size() const noexcept {
            return sizeof(m_address_in);
        }

        /**
         *
         * @return The port of this endpoint in host order
         */
        [[nodiscard]] uint16_t port() const noexcept {
            return ntohs(m_address_in.sin_port);
        }

        /**
         *
         * @return The number and dots representation of the address endpoint
         */
        [[nodiscard]] std::string address() const {
            return std::string{inet_ntoa(m_address_in.sin_addr)};
        }

        auto operator <=> (const endpoint& ep) const noexcept {
            const auto thisValue = comparison_value();
            const auto thatValue = ep.comparison_value();
            using R = decltype(thisValue <=> thatValue);
            if (thisValue < thatValue)
                return R::less;
            else if (thisValue == thatValue)
                return R::equal;
            else
                return R::greater;
        }
        auto operator == (const endpoint& ep) const noexcept {
            return m_address_in.sin_port == ep.m_address_in.sin_port && m_address_in.sin_addr.s_addr == ep.m_address_in.sin_addr.s_addr;
        }
        [[nodiscard]] uint64_t inline comparison_value() const noexcept {
            return (static_cast<uint64_t>(m_address_in.sin_addr.s_addr) << sizeof m_address_in.sin_port) + m_address_in.sin_port;
        }
    protected:
        sockaddr_in m_address_in;
    };
}
namespace std {
    template<>
    struct std::hash<sockets::endpoint>{
        std::size_t operator()(const sockets::endpoint & ep) const {
            const auto socket_hash = std::hash<uint64_t>{}(ep.comparison_value());
            return socket_hash;
        }
    };
}