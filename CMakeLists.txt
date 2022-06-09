cmake_minimum_required(VERSION 3.17)
project(JoinLess)

set(CMAKE_CXX_STANDARD 17)

find_library (PSAPI Psapi)

add_executable(JoinLess main.cpp JoinLess.cpp JoinLess.h CSVReader/CSVReader.h CSVReader/CSVReader.cpp)

target_link_libraries(JoinLess -lpsapi)