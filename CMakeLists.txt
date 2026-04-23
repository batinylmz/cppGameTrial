cmake_minimum_required(VERSION 3.20)
project(SFML_Projesi)

# SFML 3 en az C++17 standardını gerektirir
set(CMAKE_CXX_STANDARD 17)

# Apple Silicon için Homebrew kütüphane yolunu CMake'e tanıt
set(CMAKE_PREFIX_PATH "/opt/homebrew")

# SFML 3 sürümünü zorunlu olarak bul
find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(SFML_Projesi main.cpp)

# Modern CMake sözdizimi ile modülleri projeye bağla
target_link_libraries(SFML_Projesi PRIVATE SFML::Graphics SFML::Window SFML::System)