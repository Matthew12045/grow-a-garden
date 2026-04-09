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

    git clone https://github.com/Matthew12045/grow-a-garden.git
    cd grow-a-garden

**2. Switch to dev branch**

    git checkout dev

**3. Build**

🪟 Windows (PowerShell):

    mkdir build
    cd build
    cmake .. -G "MinGW Makefiles" -DSFML_DIR="C:/SFML/lib/cmake/SFML"
    mingw32-make

🐧 Linux:

    mkdir build && cd build
    cmake ..
    make

**4. Run**

🪟 Windows:

    .\GrowAGarden.exe

🐧 Linux:

    ./GrowAGarden

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

## ⚠️ Common Issues (Windows)

**SFML DLL not found on run:**
Copy all `.dll` files from `C:\SFML\bin` into the same folder as your `.exe`

**CMake can't find SFML:**
Double check `-DSFML_DIR="C:/SFML/lib/cmake/SFML"` matches your extract path

**mingw32-make not found:**
Make sure `C:\mingw64\bin` is in PATH and restart terminal after adding it

**Push rejected (repository rule violations):**
ห้าม push ตรงเข้า `dev` หรือ `main` — ต้องใช้ feature branch แล้วเปิด PR เสมอ