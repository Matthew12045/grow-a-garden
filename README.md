# 🌱 Grow A Garden

> Top-down 2D farming simulator | C++ + SFML

---

## 📋 Requirements

- [CMake](https://cmake.org/download/) >= 3.16
- [SFML](https://www.sfml-dev.org/download.php) 2.6
- [MinGW-w64](https://winlibs.com/) (Windows only)
- [Git](https://git-scm.com/download/win)

---

## ⚙️ Environment Setup

### 🪟 Windows

**1. Install CMake**

- Download from https://cmake.org/download/
- ✅ Check "Add CMake to system PATH" during install
- Verify:

      cmake --version

**2. Install MinGW-w64**

- Download from https://winlibs.com/ → grab latest GCC (UCRT, 64-bit)
- Extract to `C:\mingw64`
- Add `C:\mingw64\bin` to system PATH:
  - Search "Environment Variables" → System Variables → Path → New → `C:\mingw64\bin`
- Verify:

      gcc --version

**3. Install SFML 2.6**

- Download from https://www.sfml-dev.org/download.php
  - Choose **GCC MinGW (64-bit)**
- Extract to `C:\SFML`
- Add `C:\SFML\bin` to system PATH (same steps as above)

**4. Install Git**

- Download from https://git-scm.com/download/win
- Use default settings during install

---

### 🐧 Linux (Ubuntu/Debian)

    sudo apt update
    sudo apt install cmake g++ libsfml-dev git

---

## 🚀 Getting Started

**1. Clone the repo**

    git clone https://github.com/<your-org>/grow-a-garden.git
    cd grow-a-garden

**2. Build**

🪟 Windows (PowerShell):

    mkdir build
    cd build
    cmake .. -G "MinGW Makefiles" -DSFML_DIR="C:/SFML/lib/cmake/SFML"
    mingw32-make

🐧 Linux:

    mkdir build && cd build
    cmake ..
    make

**3. Run**

🪟 Windows:

    .\GrowAGarden.exe

🐧 Linux:

    ./GrowAGarden

---

## 🗂️ Project Structure

    grow-a-garden/
    ├── src/
    │   ├── core/          # Game loop, TickSystem
    │   ├── world/         # Garden, Cell, WeatherSystem
    │   ├── entities/      # Plant, Mutation, HarvestedItem
    │   ├── items/         # Item, Seed, Tool
    │   ├── systems/       # Shop, Inventory, RandomEvent
    │   ├── ui/            # UIManager, AudioManager
    │   └── main.cpp
    ├── assets/
    │   ├── textures/
    │   ├── audio/
    │   └── fonts/
    ├── docs/
    │   ├── class_diagram.puml
    │   └── requirements.md
    ├── CMakeLists.txt
    └── README.md

---

## 🌿 Git Workflow

    # เริ่ม feature ใหม่
    git checkout dev
    git pull
    git checkout -b feature/your-feature

    # commit
    git add .
    git commit -m "feat: your message"
    git push origin feature/your-feature

    # แล้วเปิด Pull Request → dev

### Commit message format

| Prefix | ใช้เมื่อ |
|--------|---------|
| `feat:` | เพิ่ม feature ใหม่ |
| `fix:` | แก้ bug |
| `refactor:` | ปรับโครงสร้าง code |
| `docs:` | แก้ไข documentation |
| `assets:` | เพิ่ม/แก้ไข assets |

---

## 📖 Docs

- [Class Diagram](docs/class_diagram.puml) — open with [PlantUML](https://www.plantuml.com/plantuml/)
- [Requirements](docs/requirements.md)

---

## ⚠️ Common Issues (Windows)

**SFML DLL not found on run:**
Make sure `C:\SFML\bin` is in PATH, or copy all `.dll` files from `C:\SFML\bin` into the same folder as your `.exe`

**CMake can't find SFML:**
Double check the path in cmake command matches where you extracted SFML, e.g. `-DSFML_DIR="C:/SFML/lib/cmake/SFML"`

**mingw32-make not found:**
Make sure `C:\mingw64\bin` is in PATH and restart terminal after adding it

---

## 👥 Team

| Name | Role |
|------|------|
|      |      |
|      |      |
|      |      |