cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0048 NEW)
project(part-1 VERSION 1.0.0 LANGUAGES CXX)

find_package(Qt5 COMPONENTS Core REQUIRED)

#CONFIGURE_DEPENDS requires CMake 3.12
file(GLOB SRCS CONFIGURE_DEPENDS src/*.cpp src/*.h)

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Werror)
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_11)
