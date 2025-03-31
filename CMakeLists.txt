cmake_minimum_required(VERSION 3.16)
project(JoystickMirrorControl VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
find_package(SDL2 REQUIRED)

# Add Advantech library paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/advantech/inc)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/advantech/lib)

set(PROJECT_SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/joystickmanager.cpp
    src/joystickmanager.h
    src/faststeeringmirror.cpp
    src/faststeeringmirror.h
    src/mainwindow.ui
    resources/resources.qrc
)

qt_add_executable(JoystickMirrorControl
    ${PROJECT_SOURCES}
)

target_link_libraries(JoystickMirrorControl PRIVATE 
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    SDL2::SDL2
    biodaq
)

target_include_directories(JoystickMirrorControl PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/advantech/inc
    ${CMAKE_CURRENT_BINARY_DIR}
)

install(TARGETS JoystickMirrorControl
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
