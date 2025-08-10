cmake_minimum_required(VERSION 3.21.0)
project(Dogan C)

set(CMAKE_C_STANDARD 17)

# notcurses options
set(USE_MULTIMEDIA none)
set(USE_DEFLATE off)
set(USE_DOCTEST off)
set(USE_PANDOC off)

add_subdirectory(notcurses)

add_executable(dogan src/main.c)

target_include_directories(
  dogan PRIVATE
  notcurses/include
)

target_link_libraries(dogan notcurses)
