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
    src/Presentation/SettingsManager.cpp
    src/Presentation/SettingsManager.h
    src/Presentation/ViewSettingsBinder.cpp
    src/Presentation/ViewSettingsBinder.h
    src/Presentation/WindowStateGuard.cpp
    src/Presentation/WindowStateGuard.h
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

# Deploy Qt runtime (Windows-only; copy DLLs with windeployqt)
if (WIN32)
    # Prefer the vcpkg installed dir inside the build tree when using VS integration
    set(VCPKG_INSTALLED_DIR "")
    foreach(_cand
        "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}"
        "${CMAKE_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}"
        "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}"
    )
        if (EXISTS "${_cand}")
            set(VCPKG_INSTALLED_DIR "${_cand}")
            break()
        endif()
    endforeach()


    # Locate windeployqt from vcpkg (qttools)
    set(_WINDEPLOYQT_HINTS
        "${VCPKG_INSTALLED_DIR}/tools/qt6/bin"
        "${VCPKG_INSTALLED_DIR}/tools/qt6"
        "${VCPKG_INSTALLED_DIR}/tools/qttools"
        "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/qt6/bin"
        "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/qt6"
        "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/qttools"
        "${CMAKE_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/qt6/bin"
        "${CMAKE_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/qt6"
        "${CMAKE_SOURCE_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/tools/qttools"
        "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/tools/qt6/bin"
        "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/tools/qt6"
        "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/tools/qttools"
    )
    find_program(WINDEPLOYQT_EXECUTABLE
        NAMES windeployqt windeployqt.exe
        HINTS ${_WINDEPLOYQT_HINTS}
    )

    # Resolve Debug bin/plugins with vcpkg fallbacks (installed -> packages -> buildtrees)
    set(QT_DBG_BIN_DIR "")
    foreach(_cand
        "${VCPKG_INSTALLED_DIR}/debug/bin"
        "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/bin"
        "$ENV{VCPKG_ROOT}/packages/qtbase_${VCPKG_TARGET_TRIPLET}/debug/bin"
        "$ENV{VCPKG_ROOT}/buildtrees/qtbase/${VCPKG_TARGET_TRIPLET}-dbg/bin"
    )
        if (EXISTS "${_cand}")
            set(QT_DBG_BIN_DIR "${_cand}")
            break()
        endif()
    endforeach()

    set(QT_DBG_PLUGINS_DIR "")
    foreach(_cand
        "${VCPKG_INSTALLED_DIR}/debug/plugins"
        "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/debug/plugins"
        "$ENV{VCPKG_ROOT}/packages/qtbase_${VCPKG_TARGET_TRIPLET}/debug/plugins"
        "$ENV{VCPKG_ROOT}/buildtrees/qtbase/${VCPKG_TARGET_TRIPLET}-dbg/plugins"
    )
        if (EXISTS "${_cand}")
            set(QT_DBG_PLUGINS_DIR "${_cand}")
            break()
        endif()
    endforeach()

    message(STATUS "QT_DBG_BIN_DIR='${QT_DBG_BIN_DIR}'")
    message(STATUS "QT_DBG_PLUGINS_DIR='${QT_DBG_PLUGINS_DIR}'")

    # Choose mode per configuration
    set(WINDEPLOYQT_MODE "$<$<CONFIG:Debug>:--debug>$<$<NOT:$<CONFIG:Debug>>:--release>")

    if (WINDEPLOYQT_EXECUTABLE)
        add_custom_command(TARGET PointToMesh POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Qt deploy target directory: $<TARGET_FILE_DIR:PointToMesh>"
            COMMAND ${CMAKE_COMMAND} -E echo "Running: ${WINDEPLOYQT_EXECUTABLE} ${WINDEPLOYQT_MODE} --no-translations --verbose 2 --dir \"$<TARGET_FILE_DIR:PointToMesh>\" \"$<TARGET_FILE:PointToMesh>\""
            COMMAND "${WINDEPLOYQT_EXECUTABLE}" ${WINDEPLOYQT_MODE}
                    --no-translations
                    --verbose 2
                    --dir "$<TARGET_FILE_DIR:PointToMesh>"
                    "$<TARGET_FILE:PointToMesh>"

            # Copy Debug Qt DLLs/plugins from resolved vcpkg paths (Debug config only)
            COMMAND ${CMAKE_COMMAND} -E echo "Debug DLLs fallback bin: ${QT_DBG_BIN_DIR}"
            COMMAND cmd /C "$<IF:$<CONFIG:Debug>,if not \"${QT_DBG_BIN_DIR}\"==\"\" (echo Copying Qt debug DLLs from \"${QT_DBG_BIN_DIR}\" && robocopy \"${QT_DBG_BIN_DIR}\" \"$<TARGET_FILE_DIR:PointToMesh>\" Qt6*d.dll /xo /r:1 /w:1 /np /njh /njs & if %ERRORLEVEL% LSS 8 (exit /b 0) else (exit /b %ERRORLEVEL%)) else (echo No Qt debug bin folder found), echo Skipping debug Qt copy in non-Debug configuration>"
            COMMAND ${CMAKE_COMMAND} -E echo "Debug plugins fallback dir: ${QT_DBG_PLUGINS_DIR}"
            COMMAND cmd /C "$<IF:$<CONFIG:Debug>,if not \"${QT_DBG_PLUGINS_DIR}\"==\"\" (echo Copying Qt debug plugins from \"${QT_DBG_PLUGINS_DIR}\" && robocopy \"${QT_DBG_PLUGINS_DIR}\" \"$<TARGET_FILE_DIR:PointToMesh>\\plugins\" *.dll /S /xo /r:1 /w:1 /np /njh /njs & if %ERRORLEVEL% LSS 8 (exit /b 0) else (exit /b %ERRORLEVEL%)) else (echo No Qt debug plugins folder found), echo Skipping debug plugin copy in non-Debug configuration>"

            # List DLLs to verify presence
            COMMAND ${CMAKE_COMMAND} -E echo "Listing copied DLLs under: $<TARGET_FILE_DIR:PointToMesh>"
            COMMAND ${CMAKE_COMMAND} -E chdir "$<TARGET_FILE_DIR:PointToMesh>" cmd /C "echo ==== main dir (*.dll) ==== & dir /b *.dll"
            COMMAND cmd /C "if exist \"$<TARGET_FILE_DIR:PointToMesh>\\plugins\\platforms\" (echo ==== plugins\\platforms ==== & dir /b \"$<TARGET_FILE_DIR:PointToMesh>\\plugins\\platforms\\*.dll\") else (echo plugins\\platforms folder not found)"
            COMMAND cmd /C "if exist \"$<TARGET_FILE_DIR:PointToMesh>\\plugins\\imageformats\" (echo ==== plugins\\imageformats ==== & dir /b \"$<TARGET_FILE_DIR:PointToMesh>\\plugins\\imageformats\\*.dll\") else (echo plugins\\imageformats folder not found)"
            COMMAND cmd /C "if exist \"$<TARGET_FILE_DIR:PointToMesh>\\plugins\\opengl\" (echo ==== plugins\\opengl ==== & dir /b \"$<TARGET_FILE_DIR:PointToMesh>\\plugins\\opengl\\*.dll\") else (echo plugins\\opengl folder not found)"
            COMMAND cmd /C "echo ==== check Qt6OpenGLWidgetsd.dll ==== & dir /s /b \"$<TARGET_FILE_DIR:PointToMesh>\\Qt6OpenGLWidgetsd.dll\" || echo Qt6OpenGLWidgetsd.dll not found"

            COMMENT "Deploying Qt runtime with windeployqt (copying DLLs next to the exe)"
            VERBATIM)
    else()
        message(WARNING "windeployqt not found. Ensure 'qttools' is installed via vcpkg.")
    endif()
endif()