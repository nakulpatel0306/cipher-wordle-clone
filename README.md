# Cipher - A Wordle Clone With Caesar Cipher Hints

A dark-themed Wordle clone in C++. Guess the hidden word, get colour feedback on every letter, and watch the on-screen keyboard track what you have ruled out. Each word also comes with a Caesar-shifted clue, and the key gives itself away the longer you struggle.

## At a Glance

- **Stack:** C++20, CMake, SFML 2.6, Dear ImGui, ImGui-SFML
- **Platforms:** macOS, Linux, Windows
- **State:** Complete

## Features

- Daily mode (seeded by date) and Random mode
- A Caesar cipher clue for every word, with hints that unlock as you guess: the key's parity after 2 guesses, the key itself after 3, and one revealed letter per guess from the 4th
- Green, yellow and red letter feedback, mirrored on an animated keyboard
- Board and keyboard resize with the window
- Adjustable word length, attempt count and strict dictionary check
- Session stats and win streaks

## Project Structure

```
.
├── CMakeLists.txt     # Pulls SFML, ImGui and ImGui-SFML with FetchContent
├── assets/
│   └── words.txt      # Word list, one per line (case-insensitive)
└── src/
    ├── main.cpp       # Window loop, ImGui setup, event routing
    ├── Game.hpp/.cpp  # Game state, rules, rendering and animations
    └── WordList.hpp/.cpp  # Loads words from file, falls back to a built-in list
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

Run it from the repo root so it finds `assets/words.txt`; otherwise it falls back to the built-in word list. The Windows build copies the SFML DLLs next to the executable automatically.

## Controls

- Type to fill the row, Enter to submit, Backspace to delete, Esc to quit
- **Game menu:** new random, new daily, restart the same word
- **Settings menu:** word length (4-6), attempts (4-8), strict dictionary
- **Colours:** green is the right letter in the right spot, yellow is in the word but elsewhere, red is not in the word

## License

MIT, see [LICENSE](LICENSE).
