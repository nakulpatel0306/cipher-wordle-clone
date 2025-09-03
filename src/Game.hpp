#pragma once
#include <SFML/Graphics.hpp>
#include <imgui.h>
#include <string>
#include <vector>
#include <unordered_set>
#include <random>

enum class TileState : int { Absent = 0, Present = 1, Correct = 2 };

struct Stats {
    int total = 0, wins = 0;
    int currentStreak = 0, maxStreak = 0;
    std::vector<int> distribution; // 1..N attempts
    Stats() : distribution(10, 0) {}
    void record(bool win, int attemptsUsed) {
        total++;
        if (win) {
            wins++; currentStreak++;
            if (currentStreak > maxStreak) maxStreak = currentStreak;
            if (attemptsUsed >= 1 && attemptsUsed <= (int)distribution.size())
                distribution[attemptsUsed - 1]++;
        } else {
            currentStreak = 0;
        }
    }
};

struct GameConfig {
    int length = 5;     // word length (4-6 recommended)
    int attempts = 6;   // total rows
    bool daily = false; // daily or random
};

class Game {
public:
    explicit Game();

    // lifecycle
    void newGame(const GameConfig& cfg);
    void restart();

    // input
    bool onTextEntered(sf::Uint32 unicode);
    bool onKeyPressed(sf::Keyboard::Key key);

    // logic
    void update(float dt);
    void renderUI();

    // expose
    const GameConfig& config() const { return m_cfg; }
    int currentStreak() const { return m_stats.currentStreak; }
    int maxStreak() const { return m_stats.maxStreak; }
    bool wantsToQuit() const { return m_wantsToQuit; }

private:
    // helpers
    static std::string toUpper(const std::string& s);
    static bool isAlpha(const std::string& s);
    static char caesarShiftChar(char c, int k);
    static std::string caesarShift(const std::string& s, int k);
    static uint64_t dailySeed(); // days since epoch

    void pickSecret();
    std::vector<TileState> evaluate(const std::string& guess) const;
    void pushHintProgression();

    // drawing helpers
    void drawBoard();
    void drawKeyboard();
    void topMenu();
    void footer();

private:
    GameConfig m_cfg{};
    bool m_lenient = true; // allow guesses not in dictionary
    float m_msgTimer = 0.f;
    std::string m_msg;
    std::vector<std::string> m_dict; // candidates
    std::unordered_set<std::string> m_valid; // quick membership
    std::string m_secret;

    // guesses
    std::vector<std::string> m_rows;
    std::vector<std::vector<TileState>> m_states;
    int m_rowIndex = 0;
    std::string m_current;

    // hints
    int m_caesarKey = 0;
    std::string m_cipherClue;
    std::vector<bool> m_revealMask;

    // rng
    std::mt19937_64 m_rng;

    // keyboard heatmap targets [0..2] and animated value [0..1]
    int   m_keyState[26] = {0};      // 0 absent, 1 present, 2 correct (target)
    float m_keyAnim[26]  = {0.f};    // animated brightness 0..1

    // tile flip animation per guess row
    std::vector<std::vector<float>> m_flip; // 0..1 progress per tile
    float m_flipSpeed = 6.f;                // higher = faster

    // title typing animation
    float m_titleAnimTimer = 0.f;
    int m_titleCharIndex = 0;
    bool m_titleBackspacing = false;
    std::string m_titleText = "CIPHER";

    // session stats
    Stats m_stats;

    // layout cache
    float m_tileSize = 64.f;
    float m_tileGap = 10.f;

    // status
    bool m_win = false;
    bool m_outOfAttempts = false;
    bool m_wantsToQuit = false;
};
