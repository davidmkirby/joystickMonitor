cmake_minimum_required(VERSION 3.16)
project(JoystickMonitor VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add these three important lines for Qt
set(CMAKE_AUTOMOC ON)  # Enable Qt Meta-Object Compiler
set(CMAKE_AUTORCC ON)  # Enable automatic Resource Compiler
set(CMAKE_AUTOUIC ON)  # Enable automatic UI Compiler

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(SDL2 REQUIRED)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/joystickmanager.cpp
    src/joystickmanager.h
    src/mainwindow.ui
    resources/resources.qrc
)

qt_add_executable(JoystickMonitor
    ${PROJECT_SOURCES}
)

target_link_libraries(JoystickMonitor PRIVATE 
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    SDL2::SDL2
)

target_include_directories(JoystickMonitor PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}  # Include directory for generated files
)

install(TARGETS JoystickMonitor
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
