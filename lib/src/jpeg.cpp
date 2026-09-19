#include <array>

#include <fmt/format.h>

#include "parsers.hpp"

namespace texinfo {
namespace {

constexpr std::uint8_t kMarkerPrefix = 0xff;
constexpr std::uint8_t kMarkerStartOfImage = 0xd8;
constexpr std::uint8_t kMarkerStartOfScan = 0xda;
constexpr std::uint8_t kMarkerEndOfImage = 0xd9;

bool is_standalone_marker(std::uint8_t marker) {
    return (marker >= 0xd0 && marker <= 0xd9) || marker == 0x01;
}

bool is_start_of_frame(std::uint8_t marker) {
    if (marker < 0xc0 || marker > 0xcf) return false;
    return marker != 0xc4 && marker != 0xc8 && marker != 0xcc;
}

std::uint8_t next_byte(std::istream& stream) {
    const int value = stream.get();
    if (value == std::istream::traits_type::eof()) {
        throw ProbeError("JPEG ended before a frame header was found");
    }
    return static_cast<std::uint8_t>(value);
}

std::uint8_t next_marker(std::istream& stream) {
    std::uint8_t value = next_byte(stream);
    while (value != kMarkerPrefix) {
        value = next_byte(stream);
    }
    while (value == kMarkerPrefix) {
        value = next_byte(stream);
    }
    return value;
}

Channels channels_from_component_count(std::uint8_t components) {
    switch (components) {
    case 1: return Channels::Luma;
    case 3: return Channels::Rgb;
    default: throw ProbeError(fmt::format("unsupported JPEG component count {}", components));
    }
}

}

TextureInfo parse_jpeg(std::istream& stream) {
    if (next_byte(stream) != kMarkerPrefix || next_byte(stream) != kMarkerStartOfImage) {
        throw ProbeError("missing JPEG start-of-image marker");
    }

    while (true) {
        const std::uint8_t marker = next_marker(stream);
        if (marker == kMarkerStartOfScan || marker == kMarkerEndOfImage) {
            throw ProbeError("JPEG has no frame header");
        }
        if (is_standalone_marker(marker)) {
            continue;
        }

        std::array<std::uint8_t, 2> length_field{};
        length_field[0] = next_byte(stream);
        length_field[1] = next_byte(stream);
        const std::uint32_t segment_length = read_u16_be(length_field, 0);
        if (segment_length < 2) {
            throw ProbeError("malformed JPEG segment length");
        }

        if (!is_start_of_frame(marker)) {
            stream.seekg(segment_length - 2, std::ios::cur);
            if (!stream) {
                throw ProbeError("JPEG ended before a frame header was found");
            }
            continue;
        }

        std::array<std::uint8_t, 6> frame{};
        for (std::uint8_t& field : frame) {
            field = next_byte(stream);
        }

        TextureInfo info{};
        info.container = Container::Jpg;
        info.compression = Compression::Lossy;
        info.mip_count = 1;
        info.height = read_u16_be(frame, 1);
        info.width = read_u16_be(frame, 3);
        info.channels = channels_from_component_count(frame[5]);
        return info;
    }
}

}
