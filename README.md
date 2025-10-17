# PointToMesh

This is a simple starter project using C++17, Qt, and CGAL.

This project aims to demonstrate how to configure a C++ project that uses both Qt (for the GUI) and CGAL (for computational geometry algorithms).

## 1. Prerequisites

Before you begin, ensure you have the following tools installed on your system:

-   **Git**: For cloning the project.
-   **CMake**: Version 3.16 or higher.
-   **A C++17 Compiler**:
    -   **On Windows**: Visual Studio 2022 with the "Desktop development with C++" workload is required.
    -   **On macOS**: Xcode Command Line Tools.
    -   **On Linux**: GCC or Clang.

## 2. Dependency Check (macOS & Linux)

To help you check your environment, a script is provided for macOS and Linux users.

```bash
./check_deps.sh
```
This script will check for key dependencies (like CMake, Git, and Homebrew/vcpkg) and provide guidance if they are missing.

## 3. Environment, Build, and Run Instructions

### On Windows (Recommended)

The recommended workflow on Windows is to use Visual Studio 2022, which will automatically manage dependencies using its integrated vcpkg.

1.  **Prerequisite**: Install [Visual Studio 2022](https://visualstudio.microsoft.com/vs/). Make sure to include the **"Desktop development with C++"** workload during installation.

2. **Location**: One important note: Due to Windows' path length limitations, it is recommended to place your project in a very short path such as Documents or the C: drive. Otherwise, CMake compilation may fail.

3.  **Open Project**: Launch Visual Studio 2022 and use the "Open a local folder" option to open the cloned project directory.

4.  **Automatic Dependency Installation**: Visual Studio will automatically detect the `vcpkg.json` file and start downloading and building the Qt and CGAL dependencies. You can monitor the progress in the "Output" window.

5.  **Build and Run**: Once CMake generation and dependency installation are complete, select the `PointToMesh.exe` target from the "Select Startup Item" dropdown in the toolbar and press the green "Run" button.

### On macOS (Recommended)

On macOS, the easiest way to install dependencies is using [Homebrew](https://brew.sh/).

1.  **Install Dependencies**: This project includes a `Brewfile`. Simply run the following command in the project root directory to install Qt and CGAL:
    ```bash
    brew bundle install
    ```
2.  **Build from Command Line**:
    ```bash
    # Configure
    cmake -S . -B build
    # Build
    cmake --build build
    # Run
    ./build/PointToMesh
    ```
3.  **Build with an IDE**: Open the project folder in CLion or another CMake-compatible IDE. It should automatically detect the toolchain and configure the project.

### On Linux (with vcpkg)

This workflow requires a manual installation of [vcpkg](https://vcpkg.io/).

1.  **Install vcpkg**: Follow the [official vcpkg guide](https://vcpkg.io/en/getting-started.html) to install vcpkg and set the `$VCPKG_ROOT` environment variable.

2.  **Build from Command Line**:
    ```bash
    # Configure, replacing [path/to/vcpkg] with your actual path
    cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
    # Build
    cmake --build build
    # Run
    ./build/PointToMesh
    ```
3.  **Build with an IDE**: In CLion or VS Code, configure the CMake profile to use the vcpkg toolchain file.
