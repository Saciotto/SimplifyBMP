cmake_minimum_required(VERSION 3.15)
project(simplify_bmp VERSION 1.0)
set(CMAKE_C_STANDARD 99)

set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:C,CXX,ARMClang,AppleClang,Clang,GNU>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:C,CXX,MSVC>")

add_library(compiler_flags INTERFACE)
target_compile_options(compiler_flags INTERFACE
    "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-g;-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused>>"
    "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3;-D_CRT_SECURE_NO_WARNINGS>>"
)

add_executable(simplify_bmp simplify_bmp.c bmpdispose.c bmploadfile.c bmpshowinfo.c bmpsavefile.c)
target_link_libraries(simplify_bmp compiler_flags)
