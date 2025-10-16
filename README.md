# PointToMesh

This is a simple starter project using C++17, Qt, and CGAL.

This project aims to demonstrate how to configure a C++ project that uses both Qt (for the GUI) and CGAL (for computational geometry algorithms).

## 1. Prerequisites

Before you begin, ensure you have the following basic tools installed on your system:

-   **Git**: For cloning the project.
-   **C++ Compiler**: A compiler that supports C++17 (e.g., GCC, Clang, MSVC).
-   **CMake**: Version 3.16 or higher.

You can use the detection scripts in the project to check if the dependencies are met.

## 2. Dependency Check

To help you check your environment, we provide simple detection scripts.

-   **For macOS and Linux users**:
    ```bash
    ./check_deps.sh
    ```
-   **For Windows users**:
    ```bat (must used in developer powershell for VS 2022, otherwise cl.exe won't be found)
    .\\check_deps.bat
    ```
These scripts will check for key dependencies (like CMake, Git, vcpkg, Homebrew) and provide guidance if they are missing.

## 3. Environment Setup and Dependency Installation

This project's dependencies can be managed differently depending on your operating system.

### For macOS Users (Recommended)

On macOS, the easiest way to install dependencies is using [Homebrew](https://brew.sh/).

1.  **Install Homebrew**:
    If you haven't already, install it by following the instructions on the official website.

2.  **Install Dependencies**:
    This project includes a `Brewfile`. Simply run the following command in the project root directory to install Qt and CGAL:
    ```bash
    brew bundle install
    ```

### For Windows and Linux Users (using vcpkg)

For Windows and Linux, we recommend using [vcpkg](https://vcpkg.io/), a C++ package manager that greatly simplifies library installation.

1.  **Install vcpkg**:
    Follow the [official vcpkg guide](https://vcpkg.io/en/getting-started.html) to install vcpkg. It is recommended to install it in your user directory (e.g., `C:\dev\vcpkg` or `~/vcpkg`).

2.  **Integrate with Build System (One-time setup)**:
    To allow Visual Studio and CMake to automatically discover vcpkg, run the following command:
    ```bash
    # Execute in the vcpkg directory
    ./vcpkg integrate install
    ```

3.  **Automatic Dependency Installation**:
    When you configure your project with the vcpkg toolchain, the dependencies listed in `vcpkg.json` (Qt and CGAL) will be automatically downloaded and installed.

## 4. Building and Running the Project

### On macOS (with Homebrew)

#### Using the Command Line
1.  **Configure**: `cmake -S . -B build`
2.  **Build**: `cmake --build build`
3.  **Run**: `./build/PointToMesh`

#### Using CLion
1.  Open the project folder in CLion.
2.  The IDE should automatically detect your toolchain and configure the project.
3.  Build and run the `PointToMesh` target.

### On Windows/Linux (with vcpkg)

**Important for Windows Users**: All command-line operations must be performed in a **Developer Command Prompt for Visual Studio**. You can find this in your Start Menu. Using a standard Command Prompt or PowerShell window will likely result in build failures because the C++ compiler (`cl.exe`) will not be found. A better alternative is to use the `start-vscode-dev.bat` script to launch VS Code in the correct environment.

#### Using the Command Line
1.  **Configure**: 
    ```bash
    # Select the appropriate preset for your system
    cmake --preset windows-vcpkg  # On Windows
    cmake --preset linux-vcpkg    # On Linux
    ```
2.  **Build**:
    ```bash
    # The preset name can also be used for building
    cmake --build --preset windows-vcpkg # On Windows
    cmake --build --preset linux-vcpkg   # On Linux
    ```
3.  **Run**:
    ```bash
    # Linux
    ./build/PointToMesh
    # Windows
    .\\build\\windows\\Debug\\PointToMesh.exe
    ```

#### Using an IDE
This project includes a `CMakePresets.json` file, which simplifies configuration in modern IDEs.

-   **Visual Studio 2022 / VS Code**:
    -   Ensure you have run `vcpkg integrate install` and set the `VCPKG_ROOT` environment variable.
    -   The IDE will automatically detect `CMakePresets.json`.
    -   Simply select the appropriate configure preset (e.g., `windows-vcpkg`) from the UI to configure the project.

-   **CLion**:
    -   Open the project. CLion will automatically use the presets.
    -   You can switch between presets (e.g., `windows-vcpkg`, `macos-homebrew`) using the configuration switcher in the status bar.
