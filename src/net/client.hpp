#pragma once
#include "endpoint.h"
#include <bits/functional_hash.h>
namespace net {
    struct client  {
        sockets::endpoint remote_ep{};
        uint32_t client_id;
        uint32_t packet_number;

    };
}

namespace std {
    template<>
    struct hash<sockets::endpoint>{
        std::size_t operator()(const sockets::endpoint & ep) const {
            const auto socket_hash = std::hash<uint64_t>{}(ep.comparison_value());
            return socket_hash;
        }
    };
}