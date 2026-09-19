#include "glob.hpp"

#include <algorithm>
#include <cctype>
#include <string_view>
#include <system_error>

namespace texcheck {
namespace {

char lowered(char character) {
    return static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
}

bool wildcard_match(std::string_view pattern, std::string_view text) {
    constexpr std::size_t kNoStar = std::string_view::npos;
    std::size_t pattern_index = 0;
    std::size_t text_index = 0;
    std::size_t star_index = kNoStar;
    std::size_t resume_index = 0;

    while (text_index < text.size()) {
        const bool matches_here = pattern_index < pattern.size() &&
                                  (pattern[pattern_index] == '?' ||
                                   lowered(pattern[pattern_index]) == lowered(text[text_index]));
        if (matches_here) {
            ++pattern_index;
            ++text_index;
        } else if (pattern_index < pattern.size() && pattern[pattern_index] == '*') {
            star_index = pattern_index++;
            resume_index = text_index;
        } else if (star_index != kNoStar) {
            pattern_index = star_index + 1;
            text_index = ++resume_index;
        } else {
            return false;
        }
    }
    while (pattern_index < pattern.size() && pattern[pattern_index] == '*') {
        ++pattern_index;
    }
    return pattern_index == pattern.size();
}

}

bool is_pattern(const std::string& argument) {
    return argument.find_first_of("*?") != std::string::npos;
}

std::vector<std::filesystem::path> expand_argument(const std::string& argument) {
    const std::filesystem::path path(argument);
    std::error_code status_error;
    if (std::filesystem::exists(path, status_error) || !is_pattern(argument)) {
        return {path};
    }

    std::filesystem::path directory = path.parent_path();
    if (directory.empty()) {
        directory = ".";
    }
    const std::string pattern = path.filename().string();

    std::vector<std::filesystem::path> matches;
    std::error_code iteration_error;
    for (const auto& entry : std::filesystem::directory_iterator(directory, iteration_error)) {
        if (entry.is_regular_file() && wildcard_match(pattern, entry.path().filename().string())) {
            matches.push_back(path.parent_path().empty() ? entry.path().filename() : entry.path());
        }
    }
    std::sort(matches.begin(), matches.end());
    return matches;
}

}
