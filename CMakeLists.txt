cmake_minimum_required(VERSION 3.15)
project(RELEMBRE VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(RELEMBRE src/main.cpp include/render.cpp)

target_include_directories(RELEMBRE PRIVATE include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(RELEMBRE PRIVATE ${OpenCV_LIBS})