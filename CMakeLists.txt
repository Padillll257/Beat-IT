cmake_minimum_required(VERSION 3.10)
project(BeatIT VERSION 1.0)

# Gunakan standar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cari package SFML (Graphics, Window, System, dan Audio wajib ada)
find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

# Daftarkan folder include agar header bisa terbaca
include_directories(include)

# Ambil semua file .cpp di dalam folder src
file(GLOB SOURCES "src/*.cpp")

# Buat executable game
add_executable(${PROJECT_NAME} ${SOURCES})

# ── ASSET_DIR ────────────────────────────────────────────────────────
# CMAKE_SOURCE_DIR selalu menunjuk ke folder yang berisi CMakeLists.txt
# ini (root project), terlepas dari mana "cmake .." dijalankan.
# Trailing slash disengaja, supaya di kode tinggal:
#   ASSET_DIR + "background/menu.jpg"  -> ".../assets/background/menu.jpg"
target_compile_definitions(${PROJECT_NAME} PRIVATE
    ASSET_DIR="${CMAKE_SOURCE_DIR}/assets/"
)

# Link SFML ke game kita
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system sfml-audio)