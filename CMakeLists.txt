cmake_minimum_required(VERSION 3.5)
# cmake_minimum_required(VERSION 3.4)
project(Dogan)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# notcurses options
set(USE_MULTIMEDIA none)
set(USE_DEFLATE off)
set(USE_DOCTEST off)
set(USE_PANDOC off)

if(${USE_ASAN})
  add_compile_options(-fsanitize=address)
  add_link_options(-fsanitize=address)
  add_compile_options(-Wformat -Werror=format-security)
endif()

add_subdirectory(notcurses)
add_subdirectory(yaml-cpp)

set(DOGAN_SRC
  src/main.cpp
  src/draw.cpp
  src/init.cpp
  src/config_parser.cpp
)

add_executable(dogan ${DOGAN_SRC})
target_include_directories(dogan
  PRIVATE
    include
    notcurses/include
)

target_link_libraries(dogan
  PRIVATE
    notcurses++
    yaml-cpp
)

