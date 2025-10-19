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
    find_package(CGAL CONFIG REQUIRED)
    find_package(Eigen3 CONFIG REQUIRED)
    find_package(Qt6 CONFIG REQUIRED COMPONENTS Widgets OpenGLWidgets)
else()
    find_package(CGAL REQUIRED)
    find_package(Eigen3 3.3 REQUIRED NO_MODULE)
    find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)
endif()

# Qt automoc/uic/rcc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Sources
add_executable(PointToMesh
    main.cpp
    src/UI/PointSizeControlWidget.cpp
    src/UI/PointSizeControlWidget.h
    src/UI/ColorSwatch.cpp
    src/UI/ColorSwatch.h
    src/UI/mainwindow.cpp
    src/UI/mainwindow.h
    src/UI/mainwindow.ui
    src/UI/LogPanel.cpp
    src/UI/LogPanel.h
    src/UI/ViewSettingsDialog.cpp
    src/UI/ViewSettingsDialog.h
    src/UI/ViewSettingsDialog.ui
    src/Rendering/RenderView.cpp
    src/Rendering/RenderView.h
    src/Rendering/Camera.cpp
    src/Rendering/Camera.h
    src/Rendering/Renderer.cpp
    src/Rendering/Renderer.h
    src/Rendering/ShaderLibrary.cpp
    src/Rendering/ShaderLibrary.h
    src/Presentation/ProcessingWorker.cpp
    src/Presentation/ProcessingWorker.h
    src/Presentation/PointCloudController.cpp
    src/Presentation/PointCloudController.h
        src/Settings/SettingsManager.cpp
        src/Settings/SettingsManager.h
        src/Settings/ViewSettingsBinder.cpp
        src/Settings/ViewSettingsBinder.h
        src/Settings/WindowStateGuard.cpp
        src/Settings/WindowStateGuard.h
    src/Model/Geometry.h
    src/DataProcess/CGALPointCloudProcessor.cpp
    src/DataProcess/CGALPointCloudProcessor.h
    src/DataProcess/PointCloudProcessor.h
)

# Add include directories
# Add project root so includes like "src/UI/PointSizeControlWidget.h" from UIC-generated code are found
# Also add the 'src' directory explicitly so includes like "UI/..." can be resolved via an include path.
mark_as_advanced(Qt6_INCLUDE_DIRS)
target_include_directories(PointToMesh PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
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

set(POST_BUILD_COMMANDS "")
set(POST_BUILD_COMMENTS "")

list(APPEND POST_BUILD_COMMANDS
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:PointToMesh>/resources
)
list(APPEND POST_BUILD_COMMENTS "Copying project resources...")

if (WIN32)
    # Locate windeployqt within vcpkg install paths on Windows
    find_program(WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt.exe
        HINTS
            "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/qt6/bin"
            "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/tools/qt6/bin"
    )

    if(WINDEPLOYQT_EXECUTABLE)
        list(APPEND POST_BUILD_COMMANDS
                COMMAND "${WINDEPLOYQT_EXECUTABLE}"
                $<$<CONFIG:Debug>:--debug>
                $<$<CONFIG:Release>:--release>
                --dir "$<TARGET_FILE_DIR:PointToMesh>"
                "$<TARGET_FILE:PointToMesh>"
        )
        list(APPEND POST_BUILD_COMMENTS "Deploying Qt runtime...")
    endif()
endif()

if(POST_BUILD_COMMANDS)
    add_custom_command(TARGET PointToMesh POST_BUILD
            ${POST_BUILD_COMMANDS}
            COMMENT "${POST_BUILD_COMMENTS}"
    )
endif()

# On Windows, merge <build>/vcpkg_installed/<triplet>/debug into the root prefix
if (WIN32)
    set(_vcpkg_prefix "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")

        # Generate a CMake script that performs the copy; actual copying happens only for Debug
    set(_merge_dbg_script "${CMAKE_CURRENT_BINARY_DIR}/merge_vcpkg_debug_into_root_$<CONFIG>.cmake")
    file(GENERATE OUTPUT "${_merge_dbg_script}" CONTENT "
if (\"$<CONFIG>\" STREQUAL \"Debug\")
  set(_src \"${_vcpkg_prefix}/debug\")
  set(_dst \"${_vcpkg_prefix}\")
  if (EXISTS \"\${_src}\")
        # Create directories first, then copy files using copy_if_different
    file(GLOB_RECURSE _entries RELATIVE \"\${_src}\" \"\${_src}/*\")
    foreach(rel IN LISTS _entries)
      set(_sp \"\${_src}/\${rel}\")
      set(_dp \"\${_dst}/\${rel}\")
      if (IS_DIRECTORY \"\${_sp}\")
        file(MAKE_DIRECTORY \"\${_dp}\")
      else()
        get_filename_component(_dd \"\${_dp}\" DIRECTORY)
        file(MAKE_DIRECTORY \"\${_dd}\")
        execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_if_different \"\${_sp}\" \"\${_dp}\")
      endif()
    endforeach()
  endif()
endif()
")

        # In Visual Studio builds, use PRE_BUILD to ensure the merge runs before compilation; keep windeployqt as POST_BUILD
    add_custom_command(TARGET PointToMesh PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -P $<SHELL_PATH:${_merge_dbg_script}>
        COMMENT "Merging vcpkg debug tree into ${_vcpkg_prefix} (skip identical files) for $<CONFIG>"
        VERBATIM
    )
endif()