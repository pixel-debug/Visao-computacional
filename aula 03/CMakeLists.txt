cmake_minimum_required(VERSION 3.12)
project(hough_pratos CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Localiza o OpenCV instalado no sistema
find_package(OpenCV REQUIRED)

message(STATUS "OpenCV versão: ${OpenCV_VERSION}")
message(STATUS "OpenCV include: ${OpenCV_INCLUDE_DIRS}")

add_executable(hough_pratos hough_pratos.cpp)
target_include_directories(hough_pratos PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(hough_pratos PRIVATE ${OpenCV_LIBS})
