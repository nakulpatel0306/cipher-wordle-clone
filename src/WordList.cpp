#include "WordList.hpp"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <unordered_set>

static std::string toUpper(std::string s) {
    for (auto& c : s) c = (char)std::toupper((unsigned char)c);
    return s;
}
static bool isAlpha(const std::string& s) {
    return std::all_of(s.begin(), s.end(), [](unsigned char c){ return std::isalpha(c); });
}

std::vector<std::string> WordList::loadFromFile(const std::string& path, int fixedLen) {
    std::vector<std::string> out;
    std::ifstream in(path);
    if (!in) return out;
    std::string line;
    out.reserve(10000);
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto w = toUpper(line);
        if (!isAlpha(w)) continue;
        if (fixedLen > 0 && (int)w.size() != fixedLen) continue;
        out.push_back(w);
    }
    // Deduplicate
    std::unordered_set<std::string> uniq(out.begin(), out.end());
    out.assign(uniq.begin(), uniq.end());
    return out;
}

static const char* builtin4[] = {
    "CODE","NOVA","WAVE","SIGN","NODE","BYTE","PLOT","JAZZ","TASK","RUST",
    "DATA","MESH","KERN","HASH","SYNC","PUSH","PULL","VIBE","FLOW","PLAY",
};
static const char* builtin5[] = {
    "APPLE","GRAPE","CRANE","PLANT","PIXEL","ROBOT","FRAME","LIGHT","NERVE","QUANT",
    "SOLAR","NINJA","LASER","CLOUD","STACK","SMILE","TRACK","SCALE","MUSIC","SHIFT",
    "PRIME","ALPHA","GAMMA","DELTA","SIGMA","OMEGA","MORSE","CIPHER","RHYME","BRAVE",
    "TRAIN","CHAIR","WATER","EARTH","METAL","RADIO","PIXIE","VAPOR","SWORD","BRICK",
    "CROWN","SPARK","SHINE","BLADE","NODES","ARRAY","HEART","BATCH","ENIGM","GHOST",
    "LEVEL","ROUTE","SHELL","PATCH","QUEUE","MARCH","CHART","SOUND","SWEET","TRAIL"
};
static const char* builtin6[] = {
    "VECTOR","PLAYER","PYTHON","DOCKER","SCALER","FILTER","VISION","SECRET","PUZZLE","TARGET",
    "STREAM","OBJECT","MEMORY","POCKET","SPRITE","SHADER","SCRIPT","ENGINE","MODULE","SYSTEM",
    "NUMBER","LOGGER","STATUS","BREACH","CIPHER","SIGNAL","DIRECT","METHOD","STRUCT","OUTPUT",
    "SCENES","LENGTH","EDGERS","BUFFER","STATEM","FIGURE","CHANCE","TARGET","BROWSE","BINARY"
};

std::vector<std::string> WordList::loadBuiltin(int fixedLen) {
    std::vector<std::string> out;
    if (fixedLen == 4) {
        out.assign(std::begin(builtin4), std::end(builtin4));
    } else if (fixedLen == 5) {
        out.assign(std::begin(builtin5), std::end(builtin5));
    } else if (fixedLen == 6) {
        out.assign(std::begin(builtin6), std::end(builtin6));
    } else {
        // default to 5
        out.assign(std::begin(builtin5), std::end(builtin5));
    }
    return out;
}
