cmake_minimum_required(VERSION 3.15)
project(RELEMBRE VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenCV_DIR "C:/OpenCV/build/x64/vc16/lib")
find_package(OpenCV REQUIRED)

include_directories(RELEMBRE)
include_directories("C:/OpenCV/build")

add_executable(RELEMBRE
    src/main.cpp
        include/render.cpp
        include/render.h
)

target_include_directories(RELEMBRE PRIVATE include)
target_include_directories(RELEMBRE PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(RELEMBRE PRIVATE ${OpenCV_LIBS})