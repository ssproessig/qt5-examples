cmake_minimum_required(VERSION 3.15.2 FATAL_ERROR)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)


project(QtExamples LANGUAGES CXX)



find_package(Qt5 5.15 REQUIRED COMPONENTS
    Core Network Test
    Gui Widgets
    WebEngine WebEngineWidgets
    Pdf PdfWidgets
)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)



set(COMPILE_OPTIONS)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU OR ${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
    list(APPEND COMPILE_OPTIONS -Wall -Wextra -Wpedantic)
endif()

if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
    list(APPEND COMPILE_OPTIONS
        -fdiagnostics-color=always
    )
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
    list(APPEND COMPILE_OPTIONS
        -Wformat-pedantic -fcolor-diagnostics
    )
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
    list(APPEND COMPILE_OPTIONS  
        /W4
        /experimental:external /external:anglebrackets /external:W0
        /D_CRT_SECURE_NO_WARNINGS
    )
endif()



add_subdirectory(SslUsage)
add_subdirectory(StructSerialization)
add_subdirectory(WebEnginePdf)



include(CTest)
enable_testing(true)
