#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace texcheck {

// Expands one command-line argument. The shell normally does this, but cmd.exe and PowerShell do not,
// so an unmatched pattern is expanded here instead. Returns an empty list when a pattern matches nothing.
std::vector<std::filesystem::path> expand_argument(const std::string& argument);

bool is_pattern(const std::string& argument);

}
