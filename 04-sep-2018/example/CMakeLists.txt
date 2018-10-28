cmake_minimum_required(VERSION 3.8.2)
cmake_policy(SET CMP0048 NEW)
project(part-2 VERSION 1.0.0 LANGUAGES C CXX)

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/myclass.cpp
    src/myclass.h
)

target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core Qt5::Widgets)
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Werror)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_11)
set_target_properties(${PROJECT_NAME} PROPERTIES AUTOMOC ON)
