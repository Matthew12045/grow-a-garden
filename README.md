# 🌱 Grow A Garden

> Top-down 2D farming simulator | C++ + SFML

---

## 🎮 Game Overview

**Grow A Garden** is a **single-player**, top-down 2D farming simulator for **Windows and Linux**. The player manages a small garden, buys seeds, plants crops, waits for them to grow, harvests them into a basket, and sells the harvest for **sheckles**. The game is open-ended: the goal is to keep expanding profit by choosing crops, using tools efficiently, and taking advantage of weather mutations.

Tech stack:

- **C++17**
- **SFML 3.x** for graphics, windowing, input, and audio
- **CMake** for builds
- **nlohmann/json** for save data
- **GoogleTest** for automated software tests

---

## 🕹️ How to Play

### Objective

Earn as many **sheckles** as possible by running a crop loop:

1. Buy seeds from the shop.
2. Plant seeds in empty garden cells.
3. Let crops grow over time.
4. Use tools to speed up growth when useful.
5. Harvest fully grown crops into the harvest basket.
6. Sell harvested crops from the shop's Sell tab.
7. Reinvest the sheckles into better seeds or tools.

The game has no fixed final boss or timer. It is an open-ended farming score loop where the practical "end" is the player's chosen stopping point, usually after reaching a target sheckle balance or filling the garden with high-value crops.

### Controls

| Action | Input |
|---|---|
| Start game | Click **Start** on the title screen |
| Open credits | Click **Credits** on the title screen |
| Return from credits | Press **Esc** |
| Quit from title screen | Press **Q** or close the window |
| Select seed or tool | Click an item in the inventory bar |
| Plant selected seed | Click an empty garden cell |
| Check growing crop | Click a growing crop |
| Harvest crop | Click a fully grown crop |
| Use selected tool | Select a tool, then click a planted crop |
| Open shop | Click the shop button beside the garden |
| Buy seeds/tools | Open shop, choose **Seeds** or **Tools**, then click **BUY** |
| Sell harvested crops | Open shop, choose **Sell**, then click **SELL** or **SELL ALL** |

### Rules

- The garden is a 5 columns x 4 rows board. Each cell can hold at most one plant.
- Seeds are consumed when planted. If the selected seed quantity reaches zero, it is deselected.
- Crops advance by growth ticks. Fully grown crops can be harvested.
- Some crops regrow after harvest. Regrowing crops reset to their configured regrow stage instead of being removed.
- Harvested crops go into the harvest basket first. They only become sheckles after being sold.
- The shop sells seeds and tools. Purchases fail if the player does not have enough sheckles or the inventory is full.
- The **Watering Can** advances a crop by 5 growth ticks and loses durability after a valid use.
- **Fertilizer** advances a crop by 20 growth ticks and loses durability after a valid use.
- Broken tools are removed from inventory. If another copy exists, the next tool resets to full durability.
- Weather changes over time and can speed crop growth.
- Weather can add crop mutations. Mutations increase harvest sale value:
  - Wet from Rain: x2
  - Frozen from Frost: x5
  - Shocked from Thunder Storm: x100
  - Celestial from Meteor Shower: x150
- Random events can affect the garden. The current event implementation can remove planted crops.
- The game saves player sheckles, inventory, garden plants, crop mutations, tick count, initialization state, save timestamp, and the harvest basket.
- Offline progress is applied after loading a save by comparing the save timestamp to the current time and fast-forwarding growth.

---

## 📌 Software Requirements

### Functional Requirements

- Show a title screen with start, credits, and exit behavior.
- Render a top-down garden board with visible crop growth stages.
- Allow the player to buy seed and tool items from a shop.
- Track inventory item quantities and enforce inventory capacity.
- Allow planting only when the selected seed exists and the target cell is empty.
- Grow plants over time using the tick system.
- Allow harvesting only when a crop is fully grown.
- Store harvested crops in a basket before sale.
- Sell one crop group or all basket items for sheckles.
- Apply weather effects and weather-triggered mutations to plants.
- Support tools with polymorphic behavior and durability.
- Trigger random garden events through the random event system.
- Save and load game state with JSON persistence.
- Apply offline progress when loading an older save.
- Play weather-appropriate background music and reject unsafe audio paths.

### Non-Functional Requirements

- Build with CMake on Windows and Linux.
- Use SFML 3.x for graphics, input, windowing, and audio.
- Keep asset paths relative to the project root or copied build assets.
- Use OOP encapsulation for stateful systems such as `Game`, `Garden`, `Plant`, `Inventory`, and `Shop`.
- Use inheritance and virtual dispatch for item/tool/event behavior.
- Preserve ownership safety with `std::unique_ptr<Plant>` in `Cell` and polymorphic `Item` clones.
- Keep save/load tolerant of missing or malformed optional data.
- Keep core logic testable through GoogleTest without launching the SFML window.

### OOP Talking Points

- **Class and encapsulation:** `Game`, `Garden`, `Cell`, `Plant`, `Player`, `Inventory`, `Shop`, `WeatherSystem`, and `TickSystem` keep their own state private and expose focused methods.
- **Inheritance point 1:** `Item` is the base class for `Seed`, `Tool`, and `HarvestedItem`.
- **Inheritance point 2:** `Tool` is the base class for `WateringCan` and `FertilizerTool`.
- **Inheritance point 3:** `RandomEvent` is the base class for `RaccoonEvent`.
- **Runtime state changes:** planting adds a `Plant` to a `Cell`, harvesting clears or regrows the plant, inventory quantities change after buying/planting/selling, tools lose durability, weather adds mutations, and saves restore game state.

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
    │   └── requirements.pdf
    ├── CMakeLists.txt
    └── README.md

---

## 📖 Docs

- [Class Diagram](docs/class_diagram.puml) — open with [PlantUML](https://www.plantuml.com/plantuml/)
- [Scoring Criteria](docs/requirements.pdf)

---

## 🧭 Development Notes

- `docs/class_diagram.puml` is the primary design reference for class relationships, inheritance, and associations.
- The in-game currency is called **sheckles**. Keep that spelling in code, UI text, and docs.
- The runtime garden is currently 5 columns × 4 rows: `src/ui/DrawUtils.h` defines `BOARD_COLS = 5` and `BOARD_ROWS = 4`, and `Game` constructs `Garden` from those constants.
- `Plant` is the concrete catalogue-driven crop runtime type. `Item`, `Tool`, and `RandomEvent` are abstract concepts. Do not add public constructors that allow direct instantiation of abstract base classes.
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
