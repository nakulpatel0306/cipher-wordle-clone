# Cipher - A Wordle Clone

A dark-themed Wordle clone in C++. Guess the hidden word, get colour feedback on every letter, and watch the on-screen keyboard track what you have ruled out.

## At a Glance

- **Stack:** C++20, CMake, SFML 2.6, Dear ImGui, ImGui-SFML
- **Platforms:** macOS, Linux, Windows
- **State:** Complete

## Features

- Daily mode (seeded by date) and Random mode
- Green, yellow and red letter feedback, mirrored on an animated keyboard
- Board and keyboard resize with the window
- Adjustable word length, attempt count and strict dictionary check
- Session stats and win streaks

## Project Structure

```
.
├── CMakeLists.txt     # Pulls SFML, ImGui and ImGui-SFML with FetchContent
├── assets/
│   └── words.txt      # Word list, uppercase, one per line
└── src/
    ├── main.cpp       # Window loop, ImGui setup, event routing
    ├── Game.hpp/.cpp  # Game state, rules, rendering and animations
    └── WordList.hpp/.cpp  # Loads words from file or the built-in list
```

## Running Locally

Needs CMake 3.21+, a C++20 compiler (AppleClang, MSVC 2022, GCC 11+ or Clang 13+), and internet on the first build so CMake can fetch SFML and ImGui.

macOS and Linux:

```bash
cmake --fresh -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/cipher
```

Windows (x64 Native Tools for VS 2022):

```bat
cmake --fresh -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
build\Release\cipher.exe
```

The Windows build copies the SFML DLLs next to the executable automatically.

## Controls

- Type to fill the row, Enter to submit, Backspace to delete, Esc to quit
- **Game menu:** new random, new daily, restart the same word
- **Settings menu:** word length, attempts, strict dictionary
- **Colours:** green is the right letter in the right spot, yellow is in the word but elsewhere, red is not in the word
