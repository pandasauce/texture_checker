#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <optional>
#include <system_error>
#include <vector>

#include <fmt/format.h>

#include "parsers.hpp"

namespace texinfo {
namespace {

constexpr std::size_t kHeaderPrefixSize = 148;

std::string lowercase_extension(const std::filesystem::path& file) {
    std::string extension = file.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return extension;
}

Container container_from_extension(const std::string& extension) {
    if (extension == ".dds") return Container::Dds;
    if (extension == ".png") return Container::Png;
    if (extension == ".jpg" || extension == ".jpeg") return Container::Jpg;
    if (extension == ".bmp") return Container::Bmp;
    if (extension == ".tga") return Container::Tga;
    throw ProbeError(fmt::format("unrecognised extension '{}'", extension));
}

bool starts_with(Bytes prefix, std::initializer_list<std::uint8_t> magic) {
    return prefix.size() >= magic.size() && std::equal(magic.begin(), magic.end(), prefix.begin());
}

// TGA has no magic bytes, so it is absent here and its parser sanity-checks the header instead.
std::optional<Container> sniff_container(Bytes prefix) {
    if (starts_with(prefix, {0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a})) return Container::Png;
    if (starts_with(prefix, {'D', 'D', 'S', ' '})) return Container::Dds;
    if (starts_with(prefix, {0xff, 0xd8})) return Container::Jpg;
    if (starts_with(prefix, {'B', 'M'})) return Container::Bmp;
    return std::nullopt;
}

std::vector<std::uint8_t> read_prefix(std::istream& stream) {
    std::vector<std::uint8_t> prefix(kHeaderPrefixSize);
    stream.read(reinterpret_cast<char*>(prefix.data()), kHeaderPrefixSize);
    prefix.resize(static_cast<std::size_t>(stream.gcount()));
    return prefix;
}

TextureInfo probe_by_extension(const std::filesystem::path& file) {
    const Container container = container_from_extension(lowercase_extension(file));

    std::ifstream stream(file, std::ios::binary);
    if (!stream) {
        throw ProbeError("cannot open file");
    }

    const std::vector<std::uint8_t> prefix = read_prefix(stream);
    const std::optional<Container> sniffed = sniff_container(prefix);
    if (sniffed && *sniffed != container) {
        throw ProbeError(fmt::format("extension says {} but the contents are {}",
                                     container_name(container), container_name(*sniffed)));
    }

    switch (container) {
    case Container::Dds: return parse_dds(prefix);
    case Container::Png: return parse_png(prefix);
    case Container::Bmp: return parse_bmp(prefix);
    case Container::Tga: return parse_tga(prefix);
    case Container::Jpg:
        stream.clear();
        stream.seekg(0);
        return parse_jpeg(stream);
    }
    throw ProbeError("unrecognised container");
}

}

TextureInfo probe(const std::filesystem::path& file) {
    TextureInfo info;
    try {
        info = probe_by_extension(file);
    } catch (const ProbeError& error) {
        throw ProbeError(fmt::format("{}: {}", file.string(), error.what()));
    }

    std::error_code size_error;
    info.size_bytes = std::filesystem::file_size(file, size_error);
    if (size_error) {
        throw ProbeError(fmt::format("{}: cannot determine file size", file.string()));
    }
    return info;
}

}
