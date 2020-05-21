cmake_minimum_required(VERSION 3.10.0 FATAL_ERROR)

project(cpptest VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)

set(PROJECT_UI 
    mainwindow.ui)

set(PROJECT_HEADERS
    mainwindow.h)

set(PROJECT_SOURCES 
    main.cpp
    mainwindow.cpp)

qt5_wrap_ui(PROJECT_HEADERS_WRAPPED ${PROJECT_UI})
qt5_wrap_cpp(PROJECT_SOURCES_MOC ${PROJECT_HEADERS})

add_executable(${PROJECT_NAME} ${PROJECT_HEADERS} ${PROJECT_SOURCES} 
    ${PROJECT_SOURCES_MOC} ${PROJECT_HEADERS_WRAPPED})

target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets)
