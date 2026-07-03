cmake_minimum_required(VERSION 3.21)

project(QML_ML_Camera VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Sql Svg Multimedia)

qt_standard_project_setup()

# include(CTest) defines BUILD_TESTING (default ON) and calls enable_testing(),
# so `ctest` in the build dir runs the suite under tests/.
include(CTest)

add_subdirectory(logger-core)
add_subdirectory(spectro-core)
add_subdirectory(spectro-sim)
add_subdirectory(processor-api)
add_subdirectory(camera-core)
add_subdirectory(plugins/grayscale)
add_subdirectory(app)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
