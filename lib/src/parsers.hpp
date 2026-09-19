#pragma once

#include <cstddef>
#include <cstdint>
#include <istream>
#include <span>

#include "texinfo/texinfo.hpp"

namespace texinfo {

using Bytes = std::span<const std::uint8_t>;

std::uint8_t read_u8(Bytes bytes, std::size_t offset);
std::uint16_t read_u16_le(Bytes bytes, std::size_t offset);
std::uint32_t read_u32_le(Bytes bytes, std::size_t offset);
std::uint16_t read_u16_be(Bytes bytes, std::size_t offset);
std::uint32_t read_u32_be(Bytes bytes, std::size_t offset);

TextureInfo parse_dds(Bytes header);
TextureInfo parse_png(Bytes header);
TextureInfo parse_bmp(Bytes header);
TextureInfo parse_tga(Bytes header);
TextureInfo parse_jpeg(std::istream& stream);

}
