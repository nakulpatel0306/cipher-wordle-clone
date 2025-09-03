#pragma once
#include <string>
#include <vector>

namespace WordList {
    // Load from file: uppercase letters only, filter by length if >0
    std::vector<std::string> loadFromFile(const std::string& path, int fixedLen);

    // Builtin fallback lists by length (4..6)
    std::vector<std::string> loadBuiltin(int fixedLen);
}
