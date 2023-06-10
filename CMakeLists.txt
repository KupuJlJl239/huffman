cmake_minimum_required(VERSION 3.16)

project(hw-02_huffman CXX)

add_library(huffman src/huffman.cpp)
target_include_directories(huffman PUBLIC include)

add_executable( ${PROJECT_NAME} src/main.cpp )
target_link_libraries( ${PROJECT_NAME} huffman )

add_executable( ${PROJECT_NAME}_tests test/test.cpp )
target_link_libraries( ${PROJECT_NAME}_tests huffman )
target_include_directories(${PROJECT_NAME}_tests PUBLIC test)