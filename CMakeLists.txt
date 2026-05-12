cmake_minimum_required(VERSION 3.15)
project(cvm_plus_plus
    VERSION 0.1.0
    DESCRIPTION "CVM++ — a stack-based VM and compiler for a custom scripting language"
    LANGUAGES CXX
)

# -----------------------------------------------------------------------------
# Compiler settings
# -----------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Helpful for editor/clangd integration
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------------------------------------------
# Sources
# -----------------------------------------------------------------------------
set(CVM_SOURCES
    src/Value.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/Compiler.cpp
    src/VM.cpp
    src/Debug.cpp
    src/main.cpp
)

add_executable(cvm ${CVM_SOURCES})

target_include_directories(cvm PRIVATE src)

# -----------------------------------------------------------------------------
# Warnings
# -----------------------------------------------------------------------------
if(MSVC)
    target_compile_options(cvm PRIVATE /W4 /permissive-)
else()
    target_compile_options(cvm PRIVATE
        -Wall -Wextra -Wpedantic
        -Wshadow -Wnon-virtual-dtor
        -Wold-style-cast -Wcast-align
        -Wunused -Woverloaded-virtual
    )
endif()

# -----------------------------------------------------------------------------
# Install (optional — lets `cmake --install build` drop the binary somewhere)
# -----------------------------------------------------------------------------
install(TARGETS cvm RUNTIME DESTINATION bin)

message(STATUS "CVM++ configured — build type: ${CMAKE_BUILD_TYPE}")
