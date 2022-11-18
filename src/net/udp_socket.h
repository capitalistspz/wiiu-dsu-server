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
    inline msg_flags operator&(msg_flags a, msg_flags b)
    {
        return static_cast<msg_flags>(static_cast<int>(a) & static_cast<int>(b));
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

        /**
         * Binds the socket to an endpoint
         * @param local_ep endpoint to bind the socket to
         */
        void bind(const endpoint&);

        /**
        * @param buffer the buffer to store the received m_data in
        * @param length the capacity of the buffer
        * @param flags flags to alter how the receive operation behaves
        * @param out_remote_ep the remote endpoint that the data has been received from
        * @returns number of bytes received, or else returns a negative error (see errno)
        * */
        ssize_t receive_from(uint8_t *buffer, uint16_t length, msg_flags flags, sockets::endpoint &out_remote_ep) noexcept;

        /**
        * @param buffer the buffer to take the received m_data from
        * @param length the number of bytes to read from the buffer
        * @param flags flags to alter how the send operation behaves
        * @param remote_ep the remote endpoint that the data will be sent to
        * @returns number of bytes received, or else returns a negative error (see errno) if no m_data is received and the operation is non-blocking
        * */
        ssize_t send_to(uint8_t* buffer, uint16_t length, msg_flags flags, const sockets::endpoint &remote_ep) noexcept;

        /**
        * Closes the socket
        */
        void close() noexcept;

        /**
        * Shuts down socket io
        * @param shutdownType the type of socket io operation to shutdown
        */
        bool shutdown(shutdown_type shutdownType) noexcept;

// Compiler doesn't allow split declaration for these
        /**
         * Modifies socket options
         * @tparam Value the type of the socket option value
         * @param name the socket option
         * @param value value of the option to be set
         * @param whether the option was successfully set
         */
        template <typename Value>
        bool set_option(sockets::option_name name, const Value &value) noexcept {
            const auto result = ::setsockopt(socket_fd, SOL_SOCKET, (int)name, &value, sizeof(value));
            return (result == 0);
        }
        /**
         * Retrieves the value of socket option
         * @tparam Value the type of the socket option value
         * @param name the socket option
         * @param outValue the value
         * @returns whether getting the option was a success
         */
        template <typename Value>
        bool get_option(sockets::option_name name, Value &outValue) const noexcept {
            const auto result = ::getsockopt(socket_fd, SOL_SOCKET, (int)name, outValue, sizeof(outValue));
            return (result == 0);
        }
    private:
        int socket_fd;
    };

}