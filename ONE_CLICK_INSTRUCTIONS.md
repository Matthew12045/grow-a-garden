# Grow A Garden One-Click Instructions

Send this to a friend who wants to run the game on Windows.

## What They Need First

Install these before running the game:

- Git: https://git-scm.com/download/win
- CMake: https://cmake.org/download/
- A C++ compiler, either:
  - Visual Studio Build Tools with "Desktop development with C++", or
  - MinGW-w64

## How To Run

1. Download or clone the project.
2. Open the project folder.
3. Double-click `1-click.bat`.
4. Wait for the first build to finish.
5. The game opens automatically when the build succeeds.

The first run can take a few minutes because CMake downloads and builds SFML and the other dependencies.

## Important Notes

- Keep `1-click.bat` in the project root.
- Run it from the same folder as `CMakeLists.txt` and the `assets` folder.
- If the black window shows an error, make sure CMake and a C++ compiler are installed and available on PATH.
- After the first successful build, future launches are faster.
