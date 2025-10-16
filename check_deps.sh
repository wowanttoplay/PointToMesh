#!/bin/bash

# ANSI Color Codes
GREEN='\\033[0;32m'
RED='\\033[0;31m'
YELLOW='\\033[1;33m'
NC='\\033[0m' # No Color

echo "--- Dependency Checker ---"
echo "Checking for required tools..."

# Function to check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# 1. Check for Git
echo -n "Checking for git... "
if command_exists git; then
    echo -e "${GREEN}OK${NC}"
else
    echo -e "${RED}Not Found${NC}"
    echo -e "${YELLOW}Please install Git. Visit https://git-scm.com/downloads${NC}"
fi

# 2. Check for CMake
echo -n "Checking for cmake... "
if command_exists cmake; then
    echo -e "${GREEN}OK${NC}"
else
    echo -e "${RED}Not Found${NC}"
    echo -e "${YELLOW}Please install CMake. Visit https://cmake.org/download/${NC}"
fi

# 3. Check for a C++ compiler
echo -n "Checking for C++ compiler (g++ or clang++)... "
if command_exists g++ || command_exists clang++; then
    echo -e "${GREEN}OK${NC}"
else
    echo -e "${RED}Not Found${NC}"
    echo -e "${YELLOW}Please install a C++ compiler (like GCC or Clang).${NC}"
    echo -e "${YELLOW}On Debian/Ubuntu: sudo apt-get install build-essential${NC}"
    echo -e "${YELLOW}On macOS: xcode-select --install${NC}"
fi

echo ""
echo "--- Package Manager Checks ---"
echo "This project uses vcpkg (recommended) or Homebrew (macOS) for dependencies."

# 4. Check for vcpkg
echo -n "Checking for vcpkg... "
if [ -n "$VCPKG_ROOT" ] && [ -f "$VCPKG_ROOT/vcpkg" ]; then
    echo -e "${GREEN}OK${NC} (Found in VCPKG_ROOT)"
elif [ -d "$HOME/vcpkg" ] && [ -f "$HOME/vcpkg/vcpkg" ]; then
    echo -e "${GREEN}OK${NC} (Found in $HOME/vcpkg)"
else
    echo -e "${RED}Not Found${NC}"
    echo -e "${YELLOW}vcpkg not found in common locations. If you plan to use it,${NC}"
    echo -e "${YELLOW}please install it from https://vcpkg.io and set the VCPKG_ROOT environment variable.${NC}"
fi

# 5. Check for Homebrew (macOS only)
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo -n "Checking for Homebrew (for macOS users)... "
    if command_exists brew; then
        echo -e "${GREEN}OK${NC}"
    else
        echo -e "${RED}Not Found${NC}"
        echo -e "${YELLOW}Homebrew not found. If you plan to use it, install it from https://brew.sh${NC}"
    fi
fi

echo ""
echo "--- Check Complete ---"
echo "Please review the messages above and install any missing tools."

