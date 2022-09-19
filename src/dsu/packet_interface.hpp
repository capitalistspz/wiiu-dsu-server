#pragma once

#include "../utils/reader.hpp"
#include "../utils/writer.hpp"

struct PacketData{
    virtual void swap_member_endian() = 0;
    virtual void read(utils::reader&) = 0;

    /** Writes bytes from the structure into a writer
     * @param writer the target writer
     */
    virtual void write(utils::writer&) const = 0;
    [[nodiscard]] virtual size_t size() const = 0;
};