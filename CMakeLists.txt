cmake_minimum_required(VERSION 3.16)
project(PointToMesh)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(CGAL REQUIRED)

# Enable automatic Qt features
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

add_executable(PointToMesh main.cpp)

# Add include directories
target_include_directories(PointToMesh PRIVATE
    ${Qt6_INCLUDE_DIRS}
    ${CGAL_INCLUDE_DIRS}
)

# Link libraries to the executable
target_link_libraries(PointToMesh PRIVATE
    Qt6::Widgets
    CGAL::CGAL
)