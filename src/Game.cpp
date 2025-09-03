#include "Game.hpp"
#include "WordList.hpp"
#include <imgui-SFML.h>
#include <imgui.h>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <cstring>

namespace {
    ImVec4 colAbsent    = ImVec4(0.12f, 0.12f, 0.14f, 1.0f);
    ImVec4 colPresent   = ImVec4(0.85f, 0.65f, 0.15f, 1.0f);
    ImVec4 colCorrect   = ImVec4(0.40f, 0.85f, 0.45f, 1.0f); // Lighter green for better letter visibility
    ImVec4 colWrong     = ImVec4(0.70f, 0.20f, 0.20f, 1.0f); // Red for wrong letters
    ImVec4 colTileFrame = ImVec4(0.25f, 0.25f, 0.30f, 1.0f);
    ImVec4 colBg        = ImVec4(0.08f, 0.08f, 0.10f, 1.0f);

    const char* kbdRows[3] = {
        "QWERTYUIOP",
        "ASDFGHJKL",
        "ZXCVBNM"
    };
}

Game::Game() {
    std::random_device rd;
    m_rng.seed(rd());
    newGame(m_cfg);
}

std::string Game::toUpper(const std::string& s) {
    std::string t = s;
    for (auto& c : t) c = (char)std::toupper((unsigned char)c);
    return t;
}
bool Game::isAlpha(const std::string& s) {
    return std::all_of(s.begin(), s.end(), [](unsigned char c){ return std::isalpha(c); });
}
char Game::caesarShiftChar(char c, int k) {
    if (c < 'A' || c > 'Z') return c;
    int x = c - 'A';
    x = (x + k) % 26;
    return (char)('A' + x);
}
std::string Game::caesarShift(const std::string& s, int k) {
    std::string o; o.reserve(s.size());
    for (char c : s) o.push_back(caesarShiftChar(c, k));
    return o;
}
uint64_t Game::dailySeed() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto days = duration_cast<hours>(now.time_since_epoch()).count() / 24;
    uint64_t x = (uint64_t)days;
    x ^= (x << 23); x ^= (x >> 17); x ^= (x << 5);
    return x;
}

void Game::pickSecret() {
    if (m_dict.empty()) {
        m_dict = WordList::loadBuiltin(m_cfg.length);
    }
    if (m_cfg.daily) {
        uint64_t s = dailySeed();
        m_secret = m_dict[s % m_dict.size()];
    } else {
        std::uniform_int_distribution<size_t> d(0, m_dict.size()-1);
        m_secret = m_dict[d(m_rng)];
    }
    // setup clue + reveals
    std::uniform_int_distribution<int> kd(1, 25);
    m_caesarKey = kd(m_rng);
    m_cipherClue = caesarShift(m_secret, m_caesarKey);
    m_revealMask.assign(m_cfg.length, false);
}

void Game::newGame(const GameConfig& cfg) {
    m_cfg = cfg;
    // load dict for length
    m_dict = WordList::loadFromFile("assets/words.txt", m_cfg.length);
    if (m_dict.empty()) m_dict = WordList::loadBuiltin(m_cfg.length);
    m_valid.clear();
    for (auto& w : m_dict) m_valid.insert(w);

    // rows/states
    m_rows.assign(m_cfg.attempts, std::string(m_cfg.length, ' '));
    m_states.assign(m_cfg.attempts, std::vector<TileState>(m_cfg.length, TileState::Absent));
    m_flip.assign(m_cfg.attempts, std::vector<float>(m_cfg.length, 1.f));
    m_rowIndex = 0; m_current.clear();
    std::fill(std::begin(m_keyState), std::end(m_keyState), 0);
    for (int i=0;i<26;++i) m_keyAnim[i] = 0.f;
    m_win = false; m_outOfAttempts = false;
    m_msg.clear(); m_msgTimer = 0.f;

    pickSecret();
}

void Game::restart() {
    m_rowIndex = 0; m_current.clear();
    std::fill(std::begin(m_keyState), std::end(m_keyState), 0);
    for (int i=0;i<26;++i) m_keyAnim[i] = 0.f;
    m_win = false; m_outOfAttempts = false;
    std::fill(m_rows.begin(), m_rows.end(), std::string(m_cfg.length, ' '));
    for (auto& r : m_states) std::fill(r.begin(), r.end(), TileState::Absent);
    m_flip.assign(m_cfg.attempts, std::vector<float>(m_cfg.length, 1.f));
    // keep same secret & clue
}

std::vector<TileState> Game::evaluate(const std::string& guess) const {
    std::vector<TileState> out(m_cfg.length, TileState::Absent);
    std::vector<int> counts(26, 0);
    for (int i=0;i<m_cfg.length;++i) counts[m_secret[i]-'A']++;

    // correct
    for (int i=0;i<m_cfg.length;++i) {
        if (guess[i] == m_secret[i]) {
            out[i] = TileState::Correct;
            counts[guess[i]-'A']--;
        }
    }
    // present
    for (int i=0;i<m_cfg.length;++i) {
        if (out[i] == TileState::Absent) {
            int idx = guess[i]-'A';
            if (idx>=0 && idx<26 && counts[idx] > 0) {
                out[i] = TileState::Present;
                counts[idx]--;
            }
        }
    }
    return out;
}

bool Game::onTextEntered(sf::Uint32 uni) {
    if (m_win || m_outOfAttempts) return false;
    if (uni >= 32 && uni < 128) {
        char c = (char)uni;
        if (std::isalpha((unsigned char)c)) {
            if ((int)m_current.size() < m_cfg.length) {
                m_current.push_back((char)std::toupper((unsigned char)c));
                return true;
            }
        }
    }
    return false;
}

bool Game::onKeyPressed(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Backspace) {
        if (!m_current.empty()) {
            m_current.pop_back();
            return true;
        }
    } else if (key == sf::Keyboard::Enter || key == sf::Keyboard::Return) {
        if ((int)m_current.size() == m_cfg.length) {
            if (!m_valid.empty() && !m_valid.count(m_current) && !m_lenient) {
                m_msg = "Word not in list"; m_msgTimer = 2.0f; return false;
            }
            auto st = evaluate(m_current);
            m_states[m_rowIndex] = st;
            m_rows[m_rowIndex] = m_current;
            for (int i=0;i<m_cfg.length;++i) m_flip[m_rowIndex][i] = 0.f;
            for (int i=0;i<m_cfg.length;++i) {
                int idx = m_current[i]-'A';
                int val = (st[i]==TileState::Correct?2:(st[i]==TileState::Present?1:0));
                m_keyState[idx] = std::max(m_keyState[idx], val);
            }
            
            // Mark letters that are completely wrong (not present in the word at all)
            for (int i=0;i<m_cfg.length;++i) {
                if (st[i] == TileState::Absent) {
                    int idx = m_current[i]-'A';
                    // Check if this letter appears anywhere in the word
                    bool letterInWord = false;
                    for (int j=0;j<m_cfg.length;++j) {
                        if (m_secret[j] == m_current[i]) {
                            letterInWord = true;
                            break;
                        }
                    }
                    // If letter is not in word at all, mark it as wrong
                    if (!letterInWord) {
                        m_keyState[idx] = -1; // Use -1 to indicate wrong letter
                    }
                }
            }
            if (m_current == m_secret) {
                m_win = true;
                m_stats.record(true, m_rowIndex+1);
            } else {
                pushHintProgression();
                m_rowIndex++;
                if (m_rowIndex >= m_cfg.attempts) {
                    m_outOfAttempts = true;
                    m_stats.record(false, m_cfg.attempts);
                }
            }
            m_current.clear();
            return true;
        } else {
            m_msg = "Not enough letters"; m_msgTimer = 1.5f;
        }
    }
    return false;
}

void Game::pushHintProgression() {
    int used = m_rowIndex + 1;
    if (used >= 4) {
        std::vector<int> idx;
        for (int i=0;i<m_cfg.length;++i) if (!m_revealMask[i]) idx.push_back(i);
        if (!idx.empty()) {
            std::uniform_int_distribution<int> d(0, (int)idx.size()-1);
            m_revealMask[idx[d(m_rng)]] = true;
        }
    }
}

void Game::update(float dt) {
    if (m_msgTimer > 0.f) { m_msgTimer -= dt; if (m_msgTimer < 0.f) m_msgTimer = 0.f; }
    for (int i=0;i<26;++i) {
        float target;
        if (m_keyState[i] == 2) {
            target = 1.f; // Correct - full brightness
        } else if (m_keyState[i] == 1) {
            target = 0.7f; // Present - medium brightness
        } else if (m_keyState[i] == -1) {
            target = 0.8f; // Wrong - high brightness for red
        } else {
            target = 0.f; // Unused - no brightness
        }
        float a = m_keyAnim[i];
        a += (target - a) * (dt * 6.f);
        if (a < 0.001f) a = 0.f; if (a > 0.999f) a = target;
        m_keyAnim[i] = a;
    }
    for (int r=0; r<=m_rowIndex && r < (int)m_flip.size(); ++r) {
        for (int c=0; c<m_cfg.length; ++c) {
            float& f = m_flip[r][c];
            if (f < 1.f) f = std::min(1.f, f + dt * m_flipSpeed);
        }
    }
    
    // Update title typing animation
    m_titleAnimTimer += dt;
    
    if (m_titleAnimTimer >= 0.15f) { // Adjust speed here
        m_titleAnimTimer = 0.f;
        
        if (!m_titleBackspacing) {
            // Typing forward
            if (m_titleCharIndex < (int)m_titleText.length()) {
                m_titleCharIndex++;
            } else {
                // Wait a bit before starting to backspace
                m_titleAnimTimer = -0.5f; // Wait 0.5 seconds
            }
        } else {
            // Backspacing
            if (m_titleCharIndex > 0) {
                m_titleCharIndex--;
            } else {
                m_titleBackspacing = false;
                // Wait a bit before starting to type again
                m_titleAnimTimer = -0.3f; // Wait 0.3 seconds
            }
        }
    }
    
    // Check if we should switch direction after waiting
    if (m_titleAnimTimer < 0.f) {
        if (!m_titleBackspacing && m_titleCharIndex >= (int)m_titleText.length()) {
            // Switch to backspacing after waiting
            m_titleBackspacing = true;
            m_titleAnimTimer = 0.f;
        } else if (m_titleBackspacing && m_titleCharIndex <= 0) {
            // Switch to typing after waiting
            m_titleBackspacing = false;
            m_titleAnimTimer = 0.f;
        }
    }
}

void Game::drawBoard() {
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

    // Calculate the total board dimensions
    float boardWidth = m_cfg.length * m_tileSize + (m_cfg.length - 1) * m_tileGap;
    
    // Get the window size and center the board on the entire screen
    ImVec2 windowSize = ImGui::GetWindowSize();
    float startX = (windowSize.x - boardWidth) * 0.5f;
    
    // Center the board vertically as well
    float boardHeight = m_cfg.attempts * m_tileSize + (m_cfg.attempts - 1) * m_tileGap;
    float startY = (windowSize.y - boardHeight) * 0.5f;
    
    // Draw the animated "CIPHER" title above the board
    float titleY = startY - 80.f; // Position above the board
    ImGui::SetCursorPos(ImVec2(startX, titleY));
    
    // Style the title to make it look cool
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 1.0f, 1.0f)); // Light blue color
    ImGui::SetWindowFontScale(2.5f); // Make it larger
    
    // Display the animated title text
    std::string displayText = m_titleText.substr(0, m_titleCharIndex);
    if (!m_titleBackspacing && m_titleCharIndex < (int)m_titleText.length()) {
        displayText += "|"; // Add cursor when typing
    }
    ImGui::Text("%s", displayText.c_str());
    
    ImGui::SetWindowFontScale(1.0f); // Reset font scale
    ImGui::PopStyleColor();
    
    for (int r=0;r<m_cfg.attempts;++r) {
        for (int c=0;c<m_cfg.length;++c) {
            // Calculate absolute position for each tile relative to screen center
            float tileX = startX + c * (m_tileSize + m_tileGap);
            float tileY = startY + r * (m_tileSize + m_tileGap);
            
            // Set cursor to exact position for this tile
            ImGui::SetCursorPos(ImVec2(tileX, tileY));
            
            TileState ts = m_states[r][c];
            ImVec4 fill = colAbsent;
            if (ts == TileState::Present) fill = colPresent;
            else if (ts == TileState::Correct) fill = colCorrect;  

            float flip = (r < (int)m_flip.size() ? m_flip[r][c] : 1.f);
            float scaleY = 1.f;

            // NEW: keep letters full size during the win celebration
            if (m_win) { 
                flip = 1.f; 
                scaleY = 1.f; 
            } else {
                if (flip < 0.5f) scaleY = 1.f - (flip*2.f);
                else             scaleY = (flip - 0.5f)*2.f;
            }

            ImVec4 drawCol = (flip < 0.5f ? colAbsent : fill);

            ImGui::PushStyleColor(ImGuiCol_Button, drawCol);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, drawCol);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, drawCol);
            ImGui::PushStyleColor(ImGuiCol_Border, colTileFrame);

            ImGui::PushID(r*100 + c);
            ImGui::Button("   ", ImVec2(m_tileSize, m_tileSize));
            ImVec2 p = ImGui::GetItemRectMin();
            ImVec2 s = ImGui::GetItemRectSize();

            ImGui::PushClipRect(ImVec2(p.x, p.y), ImVec2(p.x + s.x, p.y + s.y), true);
            ImGui::SetWindowFontScale(scaleY > 0.0f ? scaleY : 0.0001f);
            
            // Get the character to display
            char ch = ' ';
            if (r == m_rowIndex) {
                // Current row - show what's being typed
                if (c < (int)m_current.size()) {
                    ch = m_current[c];
                }
            } else if (r < m_rowIndex) {
                // Previous rows - show the guessed words
                ch = m_rows[r][c];
            }
            
            // Always render the character if it's valid
            if (std::isalpha((unsigned char)ch)) {
                // Center the text in the tile
                float textWidth = 10.0f; // Approximate width of a single character
                float textHeight = 20.0f; // Approximate height of a single character
                float textX = p.x + (s.x - textWidth) * 0.5f;
                float textY = p.y + (s.y - textHeight) * 0.5f;
                
                ImGui::SetCursorScreenPos(ImVec2(textX, textY));
                
                // Use white text for better visibility on all tile colors
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.96f, 0.97f, 1.0f, 1.0f));
                ImGui::Text("%c", ch);
                ImGui::PopStyleColor();
            }
            
            ImGui::SetWindowFontScale(1.f);
            ImGui::PopClipRect();
            ImGui::PopID();

            ImGui::PopStyleColor(4);
        }
    }
    
    // Move cursor to end of board for next element
    ImGui::SetCursorPos(ImVec2(0, startY + boardHeight));
    
    ImGui::PopStyleVar(2);
}

void Game::drawKeyboard() {
    ImGui::Dummy(ImVec2(0, 8));
    
    // Calculate keyboard dimensions for centering - make keys bigger
    float keyWidth = 48.f;  // Increased from 36.f
    float keyHeight = 56.f; // Increased from 42.f
    float keyGap = 8.f;     // Increased from 6.f
    
    for (int ri=0; ri<3; ++ri) {
        const char* row = kbdRows[ri];
        int n = (int)std::strlen(row);
        
        // Calculate row width and center it
        float rowWidth = n * keyWidth + (n - 1) * keyGap;
        float availWidth = ImGui::GetContentRegionAvail().x;
        float startX = (availWidth - rowWidth) * 0.5f;
        
        // Add centering spacer
        if (startX > 0) {
            ImGui::Dummy(ImVec2(startX, 0));
            ImGui::SameLine();
        }
        
        for (int i=0;i<n;++i) {
            char ch = row[i];
            int idx = ch - 'A';
            float a = m_keyAnim[idx];
            ImVec4 target;
            if (m_keyState[idx] == 2) {
                target = colCorrect;
            } else if (m_keyState[idx] == 1) {
                target = colPresent;
            } else if (m_keyState[idx] == -1) {
                target = colWrong; // Red for wrong letters
            } else {
                target = colAbsent; // Default gray for unused letters
            }
            ImVec4 base = colAbsent;
            ImVec4 fill = ImVec4(base.x + (target.x-base.x)*a,
                                 base.y + (target.y-base.y)*a,
                                 base.z + (target.z-base.z)*a,
                                 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, fill);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, fill);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, fill);
            ImGui::Button(std::string(1, ch).c_str(), ImVec2(keyWidth, keyHeight));
            ImGui::PopStyleColor(3);
            if (i+1<n) ImGui::SameLine(0, keyGap);
        }
        if (ri < 2) ImGui::Dummy(ImVec2(0, 6)); // Increased spacing between rows
    }
}

void Game::topMenu() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("game")) {
            if (ImGui::MenuItem("new random")) {
                m_cfg.daily = false; newGame(m_cfg);
            }
            if (ImGui::MenuItem("new daily")) {
                m_cfg.daily = true; newGame(m_cfg);
            }
            if (ImGui::MenuItem("restart (same word)")) {
                restart();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("quit")) {
                m_wantsToQuit = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("settings")) {
            int len = m_cfg.length;
            if (ImGui::SliderInt("word Length", &len, 4, 6)) {
                if (len != m_cfg.length) {
                    m_cfg.length = len;
                    newGame(m_cfg);
                }
            }
            int att = m_cfg.attempts;
            if (ImGui::SliderInt("attempts", &att, 4, 8)) {
                if (att != m_cfg.attempts) {
                    m_cfg.attempts = att;
                    newGame(m_cfg);
                }
            }
            bool strict = !m_lenient;
            if (ImGui::Checkbox("strict dictionary", &strict)) {
                m_lenient = !strict;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void Game::footer() {
    ImGui::Dummy(ImVec2(0, 8)); // Add spacing after keyboard
    ImGui::Separator();
    ImGui::TextDisabled("cipher clue:");
    ImGui::SameLine();
    ImGui::Text("%s", m_cipherClue.c_str());

    int used = m_rowIndex;
    if (!m_win && !m_outOfAttempts) {
        if (used >= 2) {
            ImGui::TextDisabled("hint: key parity: %s", (m_caesarKey % 2 == 0) ? "even" : "odd");
        }
        if (used >= 3) {
            ImGui::TextDisabled("hint: key k = %d", m_caesarKey);
        }
        if (used >= 4) {
            ImGui::TextDisabled("revealed letters:");
            ImGui::SameLine();
            std::string rev;
            for (int i=0;i<m_cfg.length;++i) {
                rev.push_back(m_revealMask[i] ? m_secret[i] : '_');
                if (i+1 < m_cfg.length) rev.push_back(' ');
            }
            ImGui::Text("%s", rev.c_str());
        }
    }

    if (m_win) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 1.0f, 0.6f, 1.0f));
        ImGui::Text("wow you solved it in %d/%d!", m_rowIndex, m_cfg.attempts);
        ImGui::PopStyleColor();
    } else if (m_outOfAttempts) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.6f, 1.0f));
        ImGui::Text("sorry, you're out of attempts. the word was: %s (k=%d)", m_secret.c_str(), m_caesarKey);
        ImGui::PopStyleColor();
    }

    ImGui::TextDisabled("session: played %d, wins %d, streak %d/%d",
        m_stats.total, m_stats.wins, m_stats.currentStreak, m_stats.maxStreak);

    if (m_msgTimer > 0.f && !m_msg.empty()) {
        ImGui::Separator();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 0.6f, 0.6f, 1.f));
        ImGui::TextWrapped("%s", m_msg.c_str());
        ImGui::PopStyleColor();
    }
    
    // Add bottom spacing to prevent cutoff
    ImGui::Dummy(ImVec2(0, 10));
}

void Game::renderUI() {
    ImGui::PushStyleColor(ImGuiCol_WindowBg, colBg);
    ImGui::Begin("##root", nullptr,
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::SetWindowPos(ImVec2(0, 0));
    ImGui::SetWindowSize(ImGui::GetIO().DisplaySize);

    topMenu();
    ImGui::SetCursorPos(ImVec2(10, 26));
    ImGui::TextDisabled("mode: %s  |  streak: %d/%d", (m_cfg.daily? "daily":"random"), m_stats.currentStreak, m_stats.maxStreak);

    ImVec2 display = ImGui::GetIO().DisplaySize;
    float boardW = m_cfg.length * m_tileSize + (m_cfg.length - 1) * m_tileGap;
    float boardH = m_cfg.attempts * m_tileSize + (m_cfg.attempts - 1) * m_tileGap;
    float footerH = 260.f; // Slightly reduced to prevent cutoff
    float bottomMargin = 30.f; // Increased bottom margin for better spacing

    float topBarH = 40.f;
    float availH = display.y - topBarH - footerH - bottomMargin;
    float startY = topBarH + (availH - boardH) * 0.5f;
    if (startY < topBarH + 10.f) startY = topBarH + 10.f;
    float startX = (display.x - boardW) * 0.5f;
    if (startX < 10.f) startX = 10.f;

    ImGui::SetCursorPos(ImVec2(startX, startY));
    drawBoard();

    float footerW = 800.f; // Increased width to accommodate bigger keyboard
    ImGui::SetCursorPos(ImVec2((display.x - footerW) * 0.5f, display.y - footerH - bottomMargin));
    ImGui::BeginChild("footer", ImVec2(footerW, footerH - 10.f), true);
    drawKeyboard();
    footer();
    ImGui::EndChild();

    ImGui::End();
    ImGui::PopStyleColor();
}
