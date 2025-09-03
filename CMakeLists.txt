cmake_minimum_required(VERSION 3.21)
project(cipher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# ---------- SFML ----------
FetchContent_Declare(
  SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG 2.6.1
)
FetchContent_MakeAvailable(SFML)

# ---------- Dear ImGui (fetch and expose IMGUI_DIR) ----------
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.90.9
)
FetchContent_MakeAvailable(imgui)
# ImGui source dir becomes available at: ${imgui_SOURCE_DIR}
# ImGui-SFML expects IMGUI_DIR to point to Dear ImGui root (with imgui.h)
set(IMGUI_DIR "${imgui_SOURCE_DIR}" CACHE PATH "Dear ImGui root" FORCE)

# ---------- ImGui-SFML (don't try to find SFML/ImGui via find_package) ----------
set(IMGUI_SFML_FIND_SFML OFF CACHE BOOL "" FORCE)
set(IMGUI_SFML_FIND_IMGUI OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
  ImGui-SFML
  GIT_REPOSITORY https://github.com/eliasdaler/imgui-sfml.git
  GIT_TAG v2.6
)
FetchContent_MakeAvailable(ImGui-SFML)

add_executable(cipher
  src/main.cpp
  src/Game.cpp
  src/WordList.cpp
)
target_include_directories(cipher PRIVATE src)

target_link_libraries(cipher PRIVATE
  ImGui-SFML::ImGui-SFML
  sfml-graphics
  sfml-window
  sfml-system
)

if(APPLE)
  set_target_properties(cipher PROPERTIES MACOSX_BUNDLE FALSE)
endif()

if(WIN32)
  add_custom_command(TARGET cipher POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:sfml-graphics>
      $<TARGET_FILE:sfml-window>
      $<TARGET_FILE:sfml-system>
      $<TARGET_FILE_DIR:cipher>
  )
endif()
