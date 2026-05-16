cmake_minimum_required(VERSION 3.15)
project(RELEMBRE VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(OpenCV_DIR "C:OpenCV/build/include")

include_directories(RELEMBRE)
include_directories("C:/OpenCV/build/include")

add_executable(RELEMBRE
    src/main.cpp
    include/render.cpp
    include/render.h
)

target_link_libraries(RELEMBRE PRIVATE ${OpenCV_LIBS})