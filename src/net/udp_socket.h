#pragma once

#include <sys/socket.h>
#include <stdexcept>

#include "endpoint.h"

namespace sockets {
    enum class msg_flags {
        NONE = 0,
        OUT_OF_BAND = MSG_OOB,
        PEEK = MSG_PEEK,
        DONT_ROUTE = MSG_DONTROUTE,
        DONT_WAIT = MSG_DONTWAIT
    };

    inline msg_flags operator|(msg_flags a, msg_flags b)
    {
        return static_cast<msg_flags>(static_cast<int>(a) | static_cast<int>(b));
    }

    enum class option_name {
        REUSE_ADDRESS = SO_REUSEADDR,
        KEEPALIVE = SO_KEEPALIVE,
        BROADCAST = SO_BROADCAST,
        LINGER = SO_LINGER,
        OUT_OF_BAND_DATA_INLINE = SO_OOBINLINE,
        TCP_SELECTIVE_ACKNOWLEDGEMENT = SO_TCPSACK,
        WINDOW_SCALING = SO_WINSCALE,
        SEND_BUFFER_SIZE = SO_SNDBUF,
        RECEIVE_BUFFER_SIZE = SO_RCVBUF,
        SEND_LOW_AT = SO_SNDLOWAT,
        RECEIVE_LOW_AT = SO_RCVLOWAT,
        TYPE = SO_TYPE,
        ERROR = SO_ERROR,
        RX_DATA = SO_RXDATA,
        TX_DATA = SO_TXDATA,
        SET_NON_BLOCKING = SO_NBIO,
        SET_BLOCKING_IO = SO_BIO,
        NON_BLOCK = SO_NONBLOCK
    };
    enum class shutdown_type {
        READ = SHUT_RD,
        WRITE = SHUT_WR,
        READ_WRITE = SHUT_RDWR
    };


    class udp_socket {
    public:
        udp_socket();
        ~udp_socket();
        void bind(const endpoint&);
        ssize_t receive_from(uint8_t *buffer, uint16_t length, msg_flags flags, sockets::endpoint &out_remote_ep);
        ssize_t send_to(uint8_t* buffer, uint16_t length, msg_flags flags, const sockets::endpoint &remote_ep);

        void close();
        void shutdown(shutdown_type shutdownType);

        void set_option(sockets::option_name name);

// Compiler doesn't allow split declaration for these

        template <typename Value>
        void set_option(sockets::option_name name, const Value &value) {
            const auto result = ::setsockopt(socket_fd, SOL_SOCKET, (int)name, &value, sizeof(value));
            if (result < 0)
                throw std::runtime_error(std::strerror(errno));
        }

        template <typename Value>
        void get_option(sockets::option_name name, Value &outValue) const {
            const auto result = ::getsockopt(socket_fd, SOL_SOCKET, (int)name, outValue, sizeof(outValue));
            if (result < 0)
                throw std::runtime_error(std::strerror(errno));
        }
    private:
        int socket_fd;
    };

}