cmake_minimum_required(VERSION 3.21)

project(QML_ML_Camera VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Qml Quick Sql Svg Multimedia)

qt_standard_project_setup()

add_subdirectory(logger-core)
add_subdirectory(camera-core)
add_subdirectory(app)
