cmake_minimum_required(VERSION 3.16)
project(PointToMesh)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "$ENV{PREFIX}")

find_package(CGAL REQUIRED)
message(STATUS "CGAL_DIR='${CGAL_DIR}'")
message(STATUS "CGAL_VERSION_STRING='${CGAL_VERSION_STRING}'")
message(STATUS "CGAL_SUPPORTED_COMPONENTS='${CGAL_SUPPORTED_COMPONENTS}'")

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(CGAL  REQUIRED)

# Enable automatic Qt features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add source files
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