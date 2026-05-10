# Grow A Garden — Claude Code Project Guide

## Project Overview

Top-down 2D farming simulator built in **C++ with SFML 3.x**.
Build system: **CMake + MinGW-w64** (Windows) / **g++** (Linux).
Currency in-game is called **sheckles** — do not rename or correct this.

---

## Build Commands

### Linux
```bash
mkdir build && cd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
make
cp compile_commands.json ../
```

### Windows (PowerShell)
```powershell
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DSFML_DIR="C:/SFML/lib/cmake/SFML" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
mingw32-make
cp compile_commands.json ../
```

### Run
```bash
# Linux
./GrowAGarden

# Windows
.\GrowAGarden.exe
```

### Testing
```bash
# Build tests
cmake --build build

# Run tests
./build/tests/GrowAGardenTests
```

Always run from the project root so asset paths resolve correctly.

---

## Project Structure

```
grow-a-garden/
├── src/
│   ├── core/          # Game loop (Game.cpp/h), TickSystem
│   ├── world/         # Garden, Cell, WeatherSystem
│   ├── entities/      # Plant (abstract), Mutation, HarvestedItem
│   ├── items/         # Item (abstract), Seed, Tool
│   ├── systems/       # Shop, Inventory, RandomEvent, RaccoonEvent
│   ├── ui/            # UIManager, AudioManager
│   └── main.cpp
├── assets/
│   ├── textures/      # tiles/, plants/, events/, ui/
│   ├── audio/         # bgm/, sfx/
│   └── fonts/
├── docs/
│   ├── class_diagram.puml
│   └── requirements.md
├── .agents/skills/caveman/   # Caveman communication skill
├── CMakeLists.txt
└── CLAUDE.md
```

---

## Architecture

> **Primary design reference: [`docs/class_diagram.puml`](docs/class_diagram.puml)**
> This PlantUML file is the authoritative source of truth for all class relationships,
> inheritance hierarchies, and associations in the project. When in doubt about how
> classes relate, consult the diagram before making structural changes.
> TODO: `docs/class_diagram.puml` still says the garden is 20×20; runtime code currently uses a 5×4 board.

### Core Classes

| Class | Location | Notes |
|---|---|---|
| `Game` | `src/core/` | Root object, owns all systems |
| `TickSystem` | `src/core/` | Drives all time-based logic; supports `fastForward()` for offline progress |
| `Garden` | `src/world/` | Constructor-sized grid of `Cell` objects; currently 5 columns × 4 rows |
| `Cell` | `src/world/` | Holds 0 or 1 `Plant` via `unique_ptr` |
| `Plant` | `src/entities/` | **Abstract base** — do not instantiate directly |
| `Mutation` | `src/entities/` | Weather-triggered multipliers on sell price |
| `WeatherSystem` | `src/world/` | Controls `WeatherType`, triggers mutations |
| `Player` | `src/core/` | Owns `Inventory`, equipped `Tool`, sheckles balance |
| `Shop` | `src/systems/` | Handles buy/sell transactions |
| `RandomEventManager` | `src/systems/` | Schedules and fires `RandomEvent` subclasses |
| `UIManager` | `src/ui/` | Renders all UI; supports Thai/English/Chinese |
| `AudioManager` | `src/ui/` | Syncs BGM to current weather |

### Key Enums

```cpp
enum class WeatherType { SUMMER, RAIN, FROST, THUNDER_STORM, METEOR_SHOWER };
enum class MutationType { WET, SHOCKED, FROZEN, CELESTIAL };
enum class Language { THAI, ENGLISH, CHINESE };
```

### Mutation Multipliers

| Mutation | Trigger Weather | Price Multiplier |
|---|---|---|
| WET | RAIN | ×2 |
| FROZEN | FROST | ×5 |
| SHOCKED | THUNDER_STORM | ×100 |
| CELESTIAL | METEOR_SHOWER | ×150 |

### Important Design Rules
- `Plant::calcFinalPrice()` is **private** — folds all mutation multipliers into final sell price
- `Plant::applyWeatherEffect()` modifies `ticksPerStage` **temporarily** while weather is active
- `Game::processOfflineProgress()` compares `lastSaveTimestamp` to current time and bulk fast-forwards ticks on load
- The runtime garden is currently 5 columns × 4 rows: `src/ui/DrawUtils.h` defines `BOARD_COLS = 5` and `BOARD_ROWS = 4`, and `Game` constructs `Garden` from those constants.
- `regrowsAfterHarvest` plants reset to `regrowStage` instead of being removed

---

### Item System Design

The item hierarchy uses **polymorphism for behaviour** and a **map for inventory quantities** — do not collapse these into a single approach.

**Polymorphism** (`abstract Item` → `Seed`, `HarvestedItem`, `Tool`):
- Use `std::vector<std::unique_ptr<Item>>` for Shop's available items and item registries
- Virtual dispatch handles `getPrice()`, `getName()` etc. without knowing the concrete type
- `Tool::use(Cell, Player)` is pure virtual — each tool subclass must implement its own behaviour
- `HarvestedItem` owns `mutations` and overrides `getPrice()` — this logic belongs on the object, not in a lookup table

**Map** (`std::unordered_map<std::string, int>`):
- Use for inventory quantity tracking only (item name → count)
- Fast O(1) lookup for "does the player have enough seeds to plant?"
- Do **not** use a map to represent item behaviour — it forces ugly type-checking at every call site:

```cpp
// ❌ Wrong — never do this
if (data.type == "tool") { applyToolEffect(...); }
else if (data.type == "seed") { plantSeed(...); }

// ✅ Correct — let virtual dispatch handle it
float price = item->getPrice();
```

Concrete layout:
```cpp
// Shop / registries — polymorphic
std::vector<std::unique_ptr<Item>> availableItems_;

// Inventory quantities — map
std::unordered_map<std::string, int> quantities_;

// Item prototype registry (look up by name)
std::unordered_map<std::string, std::unique_ptr<Item>> itemRegistry_;
```


### Branch Structure
```
main
└── dev
    ├── feature/matthew-core
    ├── feature/tian-plant
    ├── feature/omsin-world
    └── feature/bua-items-ui
```

### Rules
- ❌ Never push directly to `main` or `dev`
- ✅ All changes via Pull Request → `dev`
- ✅ Requires **2 approvals** before merge
- ✅ CI build must pass before merge

### Commit Message Format
```
feat:     new feature
fix:      bug fix
refactor: structural change, no behavior change
docs:     documentation only
assets:   add/modify assets
ci:       CI/CD workflow changes
```






```

When generating code that touches shared systems (Garden, Plant, TickSystem), be aware other team members are building against these interfaces — keep public APIs stable.

---

## Coding Conventions

- File naming: `PascalCase.cpp` / `PascalCase.h`
- Member variables: `trailingUnderscore_` (e.g. `name_`, `cells_`)
- Use `std::unique_ptr<Plant>` for Cell plant ownership
- Use `std::size_t` for tick counts
- Include guards or `#pragma once` on all headers
- Abstract classes use pure virtual or protected constructors — do not add public constructors to `Plant`, `Item`, `Tool`, or `RandomEvent`

---

## Common Pitfalls

- **SFML DLL not found (Windows):** copy all `.dll` files from `C:\SFML\bin` next to the `.exe`
- **CMake can't find SFML:** verify `-DSFML_DIR="C:/SFML/lib/cmake/SFML"` matches your SFML 3 install path
- **clangd-lsp not picking up SFML headers:** make sure `compile_commands.json` exists at project root (see build commands above)
- **`Plant` has a typo in private method:** `clacPrice()` — should be `calcPrice()`. Fix when touching that file.

---

## Docs

- Class diagram: `docs/class_diagram.puml` (open with PlantUML)
- GitHub repo: https://github.com/Matthew12045/grow-a-garden
