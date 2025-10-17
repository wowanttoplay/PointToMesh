cmake_minimum_required(VERSION 3.21)
project(PointToMesh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If on Windows and the vcpkg toolchain is available, use it automatically.
if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if (DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain file")
        message(STATUS "Using vcpkg toolchain: ${CMAKE_TOOLCHAIN_FILE}")
    endif()
endif()

# Keep existing prefix path for non-Windows environments (Homebrew/Conda etc.)
list(APPEND CMAKE_PREFIX_PATH "$ENV{PREFIX}")

# Dependencies
if (WIN32)
    # With vcpkg manifest mode, prefer CONFIG packages
    find_package(CGAL CONFIG REQUIRED)
    find_package(Eigen3 CONFIG REQUIRED)
    find_package(Qt6 CONFIG REQUIRED COMPONENTS Widgets OpenGLWidgets)
else()
    # Non-Windows: keep generic find_package to work with Homebrew, etc.
    find_package(CGAL REQUIRED)
    find_package(Eigen3 3.3 REQUIRED NO_MODULE)
    find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
endif()

# Optional debug info
message(STATUS "CGAL_DIR='${CGAL_DIR}'")
message(STATUS "CGAL_VERSION_STRING='${CGAL_VERSION_STRING}'")
message(STATUS "CGAL_SUPPORTED_COMPONENTS='${CGAL_SUPPORTED_COMPONENTS}'")

# Qt automoc/uic/rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Sources
add_executable(PointToMesh
    main.cpp
    src/UI/mainwindow.cpp
    src/UI/mainwindow.h
    src/UI/mainwindow.ui
    src/DataProcess/CGALPointCloudProcessor.cpp
    src/DataProcess/CGALPointCloudProcessor.h
    src/DataProcess/PointCloudProcessor.h
)

# Add include directories
target_include_directories(PointToMesh PRIVATE
    ${Qt6_INCLUDE_DIRS}
    ${CGAL_INCLUDE_DIRS}
    ${EIGEN3_INCLUDE_DIRS}
)

# Link libraries to the executable
target_link_libraries(PointToMesh PRIVATE
    Qt6::Widgets
    Qt6::OpenGLWidgets
    CGAL::CGAL
    Eigen3::Eigen
)