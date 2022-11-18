#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-make-member-function-const"
#include "udp_socket.h"
#include "../utils/exception.hpp"

#include <cerrno>

#include <unistd.h>

namespace sockets {
    udp_socket::udp_socket(){
        socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
        if (socket_fd < 0)
            throw utils::errno_error();
    }

    void udp_socket::bind(const sockets::endpoint& local_ep){
        const auto success = ::bind(socket_fd, local_ep.data(), local_ep.size());
        if (success < 0)
            throw utils::errno_error();
    }

    ssize_t udp_socket::receive_from(uint8_t *buffer, uint16_t length, sockets::msg_flags flags, sockets::endpoint &out_remote_ep) noexcept {
        sockaddr_storage clientAddress{}; socklen_t clientAddressLength = sizeof(clientAddress);
        const auto bytes = ::recvfrom(socket_fd, buffer, length, (int)flags, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressLength);
        if (bytes < 0)
            return -errno;

        out_remote_ep = sockets::endpoint{clientAddress};
        return bytes;
    }


    ssize_t udp_socket::send_to(uint8_t *buffer, uint16_t length, sockets::msg_flags flags, const sockets::endpoint& remote_ep) noexcept {
        const auto bytes = ::sendto(socket_fd, buffer, length, (int)flags, remote_ep.data(), remote_ep.size());
        if (bytes < 0)
            return -errno;
        else
            return bytes;
    }


    void udp_socket::close() noexcept{
        ::close(socket_fd);
    }

    bool udp_socket::shutdown(shutdown_type shutdownType) noexcept{
        const auto result = ::shutdown(socket_fd, (int)shutdownType);
        return (result == 0);
    }


    udp_socket::~udp_socket() {
        close();
    }
}
