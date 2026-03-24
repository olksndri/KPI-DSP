cmake_minimum_required(VERSION 3.10.0)

project(dsp_lab VERSION 0.1.0 LANGUAGES C CXX)

file(GLOB_RECURSE MATH_SRC "math/*.cpp")
file(GLOB_RECURSE DSP_SRC "dsp/*.cpp")
file(GLOB_RECURSE UTILS_SRC "utils/*.cpp")

add_executable(dsp_lab
    main.cpp
    ${MATH_SRC}
    ${DSP_SRC}
    ${UTILS_SRC}
)


target_include_directories(dsp_lab
    PUBLIC
    dsp
    libs
    math
    utils
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SNDFILE REQUIRED IMPORTED_TARGET sndfile)

set(Python3_EXECUTABLE "${CMAKE_SOURCE_DIR}/.venv/bin/python")
# Додаємо префікс, щоб CMake шукав бібліотеки саме в venv
set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/.venv")

find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)

find_package(SDL2 REQUIRED)
target_link_libraries(dsp_lab PRIVATE SDL2::SDL2 SDL2::SDL2main)

target_link_libraries(dsp_lab PRIVATE PkgConfig::SNDFILE)
target_link_libraries(dsp_lab PRIVATE Python3::Python Python3::NumPy)
