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

add_executable(simplify_bmp src/simplify_bmp.c src/bmpdispose.c src/bmploadfile.c src/bmpshowinfo.c src/bmpsavefile.c)
target_link_libraries(simplify_bmp compiler_flags)

enable_testing()

add_test(NAME 24bits_bmp 
    COMMAND $<TARGET_FILE:simplify_bmp> rgb24.bmp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_test(NAME 32bits_bmp 
    COMMAND $<TARGET_FILE:simplify_bmp> rgb32.bmp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_test(NAME 2params 
    COMMAND $<TARGET_FILE:simplify_bmp> first.bmp second.bmp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_test(NAME verbose 
    COMMAND $<TARGET_FILE:simplify_bmp> -v verbose.bmp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
)

add_test(NAME transparency 
    COMMAND $<TARGET_FILE:simplify_bmp> transparency.bmp
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests
)
