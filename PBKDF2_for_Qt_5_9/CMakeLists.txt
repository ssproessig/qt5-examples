cmake_minimum_required(VERSION 3.15.2 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)

project(PasswordDigestor LANGUAGES CXX)

find_package(Qt5 5.9 REQUIRED COMPONENTS Core Test)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

add_executable(PasswordDigestorTests 
    PasswordDigestorTests.h
    PasswordDigestorTests.cpp
    BackportedQPasswordDigestor.h
    BackportedQPasswordDigestor.cpp
)
target_link_libraries(PasswordDigestorTests Qt5::Core Qt5::Test)
add_test(NAME PasswordDigestorTests COMMAND PasswordDigestorTests)
