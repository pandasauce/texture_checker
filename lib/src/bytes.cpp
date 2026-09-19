#include "parsers.hpp"

namespace texinfo {
namespace {

Bytes require(Bytes bytes, std::size_t offset, std::size_t count) {
    if (offset + count > bytes.size()) {
        throw ProbeError("header is truncated");
    }
    return bytes.subspan(offset, count);
}

}

std::uint8_t read_u8(Bytes bytes, std::size_t offset) {
    return require(bytes, offset, 1)[0];
}

std::uint16_t read_u16_le(Bytes bytes, std::size_t offset) {
    const Bytes field = require(bytes, offset, 2);
    return static_cast<std::uint16_t>(field[0] | field[1] << 8);
}

std::uint32_t read_u32_le(Bytes bytes, std::size_t offset) {
    const Bytes field = require(bytes, offset, 4);
    return static_cast<std::uint32_t>(field[0]) |
           static_cast<std::uint32_t>(field[1]) << 8 |
           static_cast<std::uint32_t>(field[2]) << 16 |
           static_cast<std::uint32_t>(field[3]) << 24;
}

std::uint16_t read_u16_be(Bytes bytes, std::size_t offset) {
    const Bytes field = require(bytes, offset, 2);
    return static_cast<std::uint16_t>(field[0] << 8 | field[1]);
}

std::uint32_t read_u32_be(Bytes bytes, std::size_t offset) {
    const Bytes field = require(bytes, offset, 4);
    return static_cast<std::uint32_t>(field[0]) << 24 |
           static_cast<std::uint32_t>(field[1]) << 16 |
           static_cast<std::uint32_t>(field[2]) << 8 |
           static_cast<std::uint32_t>(field[3]);
}

}
