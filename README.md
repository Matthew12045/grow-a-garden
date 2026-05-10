# 🌱 Grow A Garden

> Top-down 2D farming simulator | C++ + SFML

---

## 📋 Requirements

- [CMake](https://cmake.org/download/) >= 3.16
- [SFML](https://www.sfml-dev.org/download.php) 3.x
- X11 development libraries on Linux when building SFML from source
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

**3. Install SFML 3**

- Download from https://www.sfml-dev.org/download.php
- Choose the SFML 3 package that matches your compiler, or build SFML 3 from source
- Extract to `C:\SFML`
- Add `C:\SFML\bin` to system PATH (same steps as above)

**4. Install Git**

- Download from https://git-scm.com/download/win
- Use default settings during install

---

### 🐧 Linux (Ubuntu/Debian)

    sudo apt update
    sudo apt install cmake g++ git

This project targets SFML 3. On Ubuntu/Debian, `libsfml-dev` may still provide SFML 2.x, so prefer the project's CMake-managed SFML 3 setup or install SFML 3 manually. When CMake downloads/builds SFML 3, install the X11 development packages too:

    sudo apt install libxrandr-dev libxcursor-dev libxi-dev

---

## 🚀 Getting Started

**1. Clone the repo**

    git clone https://github.com/Matthew12045/grow-a-garden.git
    cd grow-a-garden

**2. Switch to dev branch**

    git checkout dev

**3. Build**

🪟 Windows (PowerShell):

    mkdir build
    cd build
    cmake .. -G "MinGW Makefiles" -DSFML_DIR="C:/SFML/lib/cmake/SFML" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    mingw32-make
    Copy-Item compile_commands.json ..

🐧 Linux:

    mkdir build && cd build
    cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    make
    cp compile_commands.json ..

For CI or source-built SFML 3, make sure the Linux setup includes `libxrandr-dev`, `libxcursor-dev`, and `libxi-dev`. Missing these packages causes CMake to fail while configuring SFML's window module.

**4. Run**

🪟 Windows:

    .\GrowAGarden.exe

🐧 Linux:

    ./GrowAGarden

Run the game from the project root so asset paths resolve correctly.

**5. Run tests**

    cmake --build build

🪟 Windows:

    .\build\tests\GrowAGardenTests.exe

🐧 Linux:

    ./build/tests/GrowAGardenTests

---

## 🌿 Git Workflow

### Branch Structure

    main
    └── dev
        ├── feature/matthew-core
        ├── feature/tian-plant
        ├── feature/omsin-world
        └── feature/bua-items-ui

- **main** — stable only, merge จาก dev เมื่อ milestone เสร็จ
- **dev** — branch หลักสำหรับรวม features ทั้งหมด
- **feature/xxx** — แต่ละคนทำงานบน branch ของตัวเอง

### ⚠️ Rules

- ❌ ห้าม push ตรงเข้า `main` หรือ `dev`
- ✅ ทุก feature ต้องเปิด Pull Request → `dev` เสมอ
- ✅ ต้องมี 2 approvals ก่อน merge เข้า `dev`
- ✅ CI build ต้องผ่านก่อน merge ทุกครั้ง

### Daily Workflow

**1. เริ่มทำงานใหม่ทุกวัน — sync dev ก่อนเสมอ**

    git checkout dev
    git pull origin dev

**2. สร้าง feature branch จาก dev**

    git checkout -b feature/your-name-feature

**3. ทำงาน แล้ว commit**

    git add .
    git commit -m "feat: your message"

**4. Push feature branch**

    git push origin feature/your-name-feature

**5. เปิด Pull Request บน GitHub**

- ไปที่ https://github.com/Matthew12045/grow-a-garden
- กด **Compare & pull request**
- ตั้งค่า **base:** `dev` ← **compare:** `feature/your-name-feature`
- ขอ 2 คนใน team มา approve
- รอ CI ผ่าน ✅ แล้วค่อย merge

### Commit Message Format

| Prefix | ใช้เมื่อ |
|--------|---------|
| `feat:` | เพิ่ม feature ใหม่ |
| `fix:` | แก้ bug |
| `refactor:` | ปรับโครงสร้าง code |
| `docs:` | แก้ไข documentation |
| `assets:` | เพิ่ม/แก้ไข assets |
| `ci:` | แก้ไข CI/CD workflow |

### ตัวอย่าง commit messages

    feat: add Plant abstract class
    feat: implement WeatherSystem with 5 weather types
    fix: cell setPlant null pointer crash
    refactor: move calcFinalPrice to private
    docs: update class diagram

---

## 👥 Team & Issue Assignment

| Name | Branch | Issues |
|------|--------|--------|
| Matthew | `feature/matthew-core` | #1-3, #19-20 |
| เทียน | `feature/tian-plant` | #7-10 |
| ออมสิน | `feature/aosmin-world` | #4-6, #15-16 |
| บัว | `feature/bua-items-ui` | #11-14, #17-18 |

### Build Order (สำคัญ)

    Matthew (#1-3) ← ทุกคนรอก่อน
         ↓
    เทียน + ออมสิน ← ทำพร้อมกันได้
         ↓
    บัว (#11-18) ← รอ Plant และ Garden พร้อมก่อน
         ↓
    Matthew (#19-20) ← ทำสุดท้าย

---

## 🗂️ Project Structure

    grow-a-garden/
    ├── .github/
    │   └── workflows/
    │       └── ci.yml         # GitHub Actions CI
    ├── src/
    │   ├── core/              # Game loop, TickSystem
    │   ├── world/             # Garden, Cell, WeatherSystem
    │   ├── entities/          # Plant, Mutation, HarvestedItem
    │   ├── items/             # Item, Seed, Tool
    │   ├── systems/           # Shop, Inventory, RandomEvent
    │   ├── ui/                # UIManager, AudioManager
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

## 📖 Docs

- [Class Diagram](docs/class_diagram.puml) — open with [PlantUML](https://www.plantuml.com/plantuml/)
- [Requirements](docs/requirements.md)

---

## 🧭 Development Notes

- `docs/class_diagram.puml` is the primary design reference for class relationships, inheritance, and associations.
- The in-game currency is called **sheckles**. Keep that spelling in code, UI text, and docs.
- `Garden` is always a 20×20 grid.
- `Plant`, `Item`, `Tool`, and `RandomEvent` are abstract concepts. Do not add public constructors that allow direct instantiation.
- Keep shared public APIs stable when touching `Garden`, `Plant`, `TickSystem`, or other systems that teammates build against.
- Item behavior should stay polymorphic through `Item`, `Seed`, `HarvestedItem`, and `Tool`; inventory quantities should stay map-based for item name counts.
- Member variables use trailing underscores, for example `name_` and `cells_`.

---

## ⚠️ Common Issues (Windows)

**SFML DLL not found on run:**
Copy all `.dll` files from `C:\SFML\bin` into the same folder as your `.exe`

**CMake can't find SFML:**
Double check `-DSFML_DIR="C:/SFML/lib/cmake/SFML"` matches your SFML 3 extract or install path

**clangd cannot find SFML headers:**
Make sure `compile_commands.json` exists at the project root. The build commands above copy it from the build folder.

**mingw32-make not found:**
Make sure `C:\mingw64\bin` is in PATH and restart terminal after adding it

**Push rejected (repository rule violations):**
ห้าม push ตรงเข้า `dev` หรือ `main` — ต้องใช้ feature branch แล้วเปิด PR เสมอ

---

## ⚠️ Common Issues (Linux / CI)

**CMake fails with `Could NOT find X11 (missing: Xrandr Xcursor Xi)`:**
Install the missing X11 development packages:

    sudo apt install libxrandr-dev libxcursor-dev libxi-dev

This usually happens when SFML 3 is built from source through CMake `FetchContent`. Do not rely on Ubuntu's `libsfml-dev` package unless you have confirmed it provides SFML 3 for your distribution.

**GitHub Actions warns about `actions/checkout@v3`:**
Update the workflow to `actions/checkout@v4`. The warning is not a build failure, but it should be cleaned up before GitHub's Node 20 action runtime removal.
