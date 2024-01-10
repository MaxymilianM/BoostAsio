cmake_minimum_required(VERSION 3.27)

# Required for cmake_print_variables()
include(CMakePrintHelpers)

# Set CPP version (same as "-std=c++11")
set(CMAKE_CXX_STANDARD 11)

project(echoserverasync)

set(SRC wrapper.cpp
        wrapper.h
        echoserverasync.cpp)

# Specify which boost libs to include
set(BOOST_INCLUDE_LIBRARIES thread system asio chrono)
set(BOOST_ENABLE_CMAKE ON)

# set(BOOST_ROOT "/usr/local/")
set(Boost_INCLUDE_DIR  "/usr/local/")
set(Boost_LIBRARY_DIR "/usr/local/lib")

# Find Boost package with all specified BOOST_INCLUDE_LIBRARIES libs
find_package(Boost)

IF (Boost_FOUND)
    INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
    cmake_print_variables(Boost_INCLUDE_DIR)
    cmake_print_variables(Boost_LIBRARY_DIR)
    cmake_print_variables(BOOST_INCLUDE_LIBRARIES)
    ADD_DEFINITIONS( "-DHAS_BOOST" )
ENDIF()

add_executable(${PROJECT_NAME} ${SRC})

# Link all specified Boost libs that we previously found
target_link_libraries ( ${PROJECT_NAME} boost_system boost_thread pthread ncurses boost_chrono)
