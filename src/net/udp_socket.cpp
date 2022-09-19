#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-make-member-function-const"
#include "udp_socket.h"
#include "../utils/exception.hpp"

#include <stdexcept>
#include <cerrno>
#include <cstring>

#include <unistd.h>


namespace sockets {
    udp_socket::udp_socket(){
        socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
        if (socket_fd < 0)
            throw utils::errno_error();
    }
    /**
     *
     * @param local_ep endpoint to bind the socket to
     */
    void udp_socket::bind(const sockets::endpoint& local_ep){
        const auto success = ::bind(socket_fd, local_ep.data(), local_ep.size());
        if (success < 0)
            throw utils::errno_error();
    }

    /**
     * @param buffer the buffer to store the received m_data in
     * @param length the capacity of the buffer
     * @param flags flags to alter how the receive operation behaves
     * @param out_remote_ep the remote endpoint that the data has been received from
     * @returns number of bytes received, or else returns a negative error (see errno) if no m_data is received and the operation is non-blocking
     * */
    ssize_t udp_socket::receive_from(uint8_t *buffer, uint16_t length, sockets::msg_flags flags, sockets::endpoint &out_remote_ep) {
        sockaddr_storage clientAddress{}; socklen_t clientAddressLength = sizeof(clientAddress);
        const auto bytes = ::recvfrom(socket_fd, buffer, length, (int)flags, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressLength);
        if (bytes < 0){
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return -errno;
            else
                throw utils::errno_error();
        }
        out_remote_ep = sockets::endpoint{clientAddress};
        return bytes;
    }

    /**
     * @param buffer the buffer to take the received m_data from
     * @param length the number of bytes to read from the buffer
     * @param flags flags to alter how the send operation behaves
     * @param remote_ep the remote endpoint that the data will be sent to
     * @returns number of bytes received, or else returns a negative error (see errno) if no m_data is received and the operation is non-blocking
     * */
    ssize_t udp_socket::send_to(uint8_t *buffer, uint16_t length, sockets::msg_flags flags, const sockets::endpoint& remote_ep) {
        const auto bytes = ::sendto(socket_fd, buffer, length, (int)flags, remote_ep.data(), remote_ep.size());
        if (bytes < 0)
            throw utils::errno_error();
        else
            return bytes;
    }

    /**
     * Closes the socket
     */
    void udp_socket::close(){
        ::close(socket_fd);
    }

    /**
     * @param shutdownType the type of socket io operation to shutdown
     */
    void udp_socket::shutdown(shutdown_type shutdownType){
        const auto result = ::shutdown(socket_fd, (int)shutdownType);
        if (result < 0)
            throw utils::errno_error();
    }

    void udp_socket::set_option(sockets::option_name name) {
        const auto result = ::setsockopt(socket_fd, SOL_SOCKET, (int)name, nullptr, 0);
        if (result < 0)
            throw utils::errno_error();
    }

    udp_socket::~udp_socket() {
        close();
    }
}
#pragma clang diagnostic pop