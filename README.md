# 🔐 Cipher - A Wordle Clone

A modern, dark-themed word puzzler built in **C++**. Guess the hidden word and get instant **green / yellow / red** feedback while a live **keyboard heatmap** reacts to your inputs. Jump into a fresh **Daily** challenge or spin up a **Random** run — sleek UI, responsive controls, and a focused, puzzle-first experience.

---

## ✨ Features
- Responsive board & keyboard that **auto-resize** with the window
- On-screen keyboard with animated colors:
  - **Green** = correct; **Yellow** = present; **Red** = not in word
- **Daily** mode (seeded by date) and **Random** mode
- “Win celebration”: **all tiles turn green**
- Session **stats & streaks**
- **Strict dictionary** toggle (or play lenient for quick testing)
- Minimal, aesthetic **dark UI** (ImGui)

---

## 🗂 Project Structure
```
.
├── CMakeLists.txt                 # FetchContent: SFML, ImGui, ImGui-SFML
├── assets/
│   └── words.txt                  # Optional word list (uppercase, one per line)
└── src/
    ├── main.cpp                   # Window loop, ImGui init, event routing
    ├── Game.hpp                   # Core game types, config, state, UI hooks
    ├── Game.cpp                   # Game logic, rendering, animations
    ├── WordList.hpp               # Word list loader (file + builtin)
    └── WordList.cpp
```

---

## 🚀 Getting Started

### Prereqs
- **CMake 3.21+**
- A C++ compiler with **C++20** support  
  - macOS: Xcode / AppleClang  
  - Windows: MSVC (VS 2022)  
  - Linux: GCC 11+ or Clang 13+
- Internet access the first time you configure (CMake **FetchContent** grabs SFML & ImGui)

### Build & Run (macOS / Linux)
```bash
# from the project root
cmake --fresh -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/cipher
```

### Build & Run (Windows, x64 Native Tools for VS 2022)
```bat
cmake --fresh -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
build\Release\cipher.exe
```

> Windows note: the CMake script copies required SFML runtime DLLs next to the EXE after build.

---

## 🎮 How to Play
- **Type letters** to fill the current row  
- **Enter/Return** to submit  
- **Backspace** to delete  
- **ESC** to quit  
- Top menu → **game**: _new random_ / _new daily_ / _restart (same word)_  
- Top menu → **settings**: _word length_, _attempts_, _strict dictionary_  

**Colors after submit**
- **Green**: letter is correct and in the right position  
- **Yellow**: letter exists but in a different position  
- **Red**: letter not in the secret word  

---

## 🛠 Tech Stack
- **C++20**, **CMake**
- **SFML 2.6.x**
- **Dear ImGui 1.90+**
- **ImGui-SFML v2.6**
