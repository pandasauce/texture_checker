#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

#include <argparse/argparse.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "glob.hpp"
#include "texinfo/texinfo.hpp"

namespace {

struct Selection {
    bool format = true;
    bool channels = false;
    bool compression = false;
    bool resolution = false;
    bool mips = false;
    bool size = false;
    bool exact_bytes = false;
};

bool any_dataset_selected(const Selection& selection) {
    return selection.format || selection.channels || selection.compression ||
           selection.resolution || selection.mips || selection.size;
}

// Binary mode stops the Windows CRT from rewriting every '\n' as "\r\n" and littering pipelines with '\r'.
void use_lf_line_endings() {
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stderr), _O_BINARY);
#endif
}

void route_diagnostics_to_stderr() {
    spdlog::set_default_logger(spdlog::stderr_color_mt("texcheck"));
    spdlog::set_formatter(std::make_unique<spdlog::pattern_formatter>(
        "%^%l:%$ %v", spdlog::pattern_time_type::local, std::string("\n")));
}

void configure_parser(argparse::ArgumentParser& program) {
    program.add_description("Report format, channel, compression, resolution, mip and size datasets for textures. "
                            "The format is shown unless switched off; the rest are opt-in.");
    program.add_argument("files").remaining().help("texture files or glob patterns");
    program.add_argument("--no-format").flag().help("omit the container format, which is otherwise always shown");
    program.add_argument("--channels").flag().help("add the channel layout dataset");
    program.add_argument("--compression").flag().help("add the compression dataset");
    program.add_argument("--resolution").flag().help("add the pixel resolution dataset");
    program.add_argument("--mips").flag().help("add the mip level count dataset");
    program.add_argument("--size").flag().help("add the file size dataset");
    program.add_argument("--bytes").flag().help("render sizes as exact byte counts instead of KiB/MiB");
}

Selection read_selection(const argparse::ArgumentParser& program) {
    Selection selection;
    selection.format = !program.get<bool>("--no-format");
    selection.channels = program.get<bool>("--channels");
    selection.compression = program.get<bool>("--compression");
    selection.resolution = program.get<bool>("--resolution");
    selection.mips = program.get<bool>("--mips");
    selection.size = program.get<bool>("--size");
    selection.exact_bytes = program.get<bool>("--bytes");
    return selection;
}

std::string build_line(const texinfo::TextureInfo& info, const Selection& selection) {
    std::vector<std::string> datasets;
    if (selection.format) datasets.emplace_back(texinfo::container_name(info.container));
    if (selection.channels) datasets.emplace_back(texinfo::channels_name(info.channels));
    if (selection.compression) datasets.emplace_back(texinfo::compression_name(info.compression));
    if (selection.resolution) datasets.push_back(texinfo::resolution_text(info));
    if (selection.mips) datasets.push_back(texinfo::mip_text(info));
    if (selection.size) datasets.push_back(texinfo::size_text(info, selection.exact_bytes));
    return fmt::format("{}", fmt::join(datasets, " | "));
}

bool report(const std::filesystem::path& file, const Selection& selection) {
    const std::string name = file.generic_string();
    try {
        const texinfo::TextureInfo info = texinfo::probe(file);
        if (info.channels == texinfo::Channels::Indexed) {
            spdlog::warn("{}: indexed colour, which the GPU cannot sample directly", name);
        }
        fmt::print("{}: {}\n", name, build_line(info, selection));
        return true;
    } catch (const texinfo::ProbeError& error) {
        spdlog::error(error.what());
        return false;
    }
}

}

int main(int argc, char** argv) {
    use_lf_line_endings();
    route_diagnostics_to_stderr();

    argparse::ArgumentParser program("texcheck", "0.1.0");
    configure_parser(program);
    try {
        program.parse_args(argc, argv);
    } catch (const std::exception& error) {
        spdlog::error(error.what());
        return 2;
    }

    std::vector<std::string> arguments;
    if (program.is_used("files")) {
        arguments = program.get<std::vector<std::string>>("files");
    }
    if (arguments.empty()) {
        spdlog::error("no input files");
        return 2;
    }

    const Selection selection = read_selection(program);
    if (!any_dataset_selected(selection)) {
        spdlog::error("--no-format left no datasets to report");
        fmt::print(stderr, "{}", program.help().str());
        return 2;
    }

    bool everything_succeeded = true;
    for (const std::string& argument : arguments) {
        const std::vector<std::filesystem::path> files = texcheck::expand_argument(argument);
        if (files.empty()) {
            spdlog::error("{}: no files matched", argument);
            everything_succeeded = false;
            continue;
        }
        for (const std::filesystem::path& file : files) {
            everything_succeeded &= report(file, selection);
        }
    }
    return everything_succeeded ? 0 : 1;
}
