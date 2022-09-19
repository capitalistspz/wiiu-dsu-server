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
        endpoint(std::string_view address, uint16_t port_number) : m_address_in() {
            m_address_in.sin_family = AF_INET;
            const auto result = inet_aton(address.data(), &m_address_in.sin_addr);
            if (result == 0)
                throw std::invalid_argument("address");
            m_address_in.sin_port = htons(port_number);
        }
        endpoint(uint32_t address, uint16_t port_number) : m_address_in() {
            m_address_in.sin_family = AF_INET;
            m_address_in.sin_addr.s_addr = htonl(address);
            m_address_in.sin_port = htons(port_number);
        }

        explicit endpoint(const sockaddr_in& addr)
        :m_address_in(addr){}

        endpoint(const sockaddr& addr, size_t size) : m_address_in(){
            std::memcpy(&m_address_in, &addr, size);
        }

        explicit endpoint(const sockaddr_storage& storage){
            std::memcpy(&m_address_in, &storage, sizeof(storage));
        }

        /**
         * Creates an invalid endpoint
         */
        endpoint() = default;

        endpoint(endpoint const &ep) = default;

        endpoint(endpoint&& ep) noexcept = default;


        endpoint& operator=(const endpoint& rhs)= default;

        endpoint& operator=(endpoint&& rhs) noexcept = default;

        /**
         * @return The capacity of the underlying socket endpoint
         */
        [[nodiscard]] const sockaddr* data() const {
            return (sockaddr*)&m_address_in;
        }

        /**
         * @return The capacity of the underlying socket endpoint
         */
        [[nodiscard]] size_t size() const {
            return sizeof(m_address_in);
        }

        /**
         *
         * @return The port of this endpoint in host order
         */
        [[nodiscard]] uint16_t port() const {
            return ntohs(m_address_in.sin_port);
        }

        /**
         *
         * @return The number and dots representation of the address endpoint
         */
        [[nodiscard]] const char* address() const {
            return inet_ntoa(m_address_in.sin_addr);
        }

        auto operator <=> (const endpoint& ep) const{
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
        auto operator == (const endpoint& ep) const {
            return m_address_in.sin_port == ep.m_address_in.sin_port && m_address_in.sin_addr.s_addr == ep.m_address_in.sin_addr.s_addr;
        }
        [[nodiscard]] uint64_t inline comparison_value() const {
            return (static_cast<uint64_t>(m_address_in.sin_addr.s_addr) << 2) + m_address_in.sin_port;
        }
    protected:
        sockaddr_in m_address_in;
    };
}