# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**solid-broccoli** is a Qt6/QML tricorder-style multi-sensor instrument: live spectroscopy (simulated instruments + BLE bridge path), a Geiger-counter modality, a SQLite lab notebook (sessions, captures, measurements, reports), a documentation camera with photo/video albums and a spectrum-overlay recorder, an optional ONNX object-detection plugin, and an optional on-device AI report generator (llama.cpp + Gemma). The full architecture is documented in `docs/ARCHITECTURE.md` (canonical, Mermaid) — read it before structural changes; `docs/USER_MANUAL.md` describes behavior from the user's side.

## Build

The project uses CMake (migrated from qmake). The build directory is `QML_ML_Camera/build/`.

```bash
# Configure (from repo root)
cmake -S QML_ML_Camera -B QML_ML_Camera/build

# Build
cmake --build QML_ML_Camera/build -j

# Run
./QML_ML_Camera/build/app/QML_ML_Camera_App.app/Contents/MacOS/QML_ML_Camera_App

# Tests and QML lint
ctest --test-dir QML_ML_Camera/build --output-on-failure
/opt/homebrew/opt/qt/bin/qmllint QML_ML_Camera/app/*.qml
```

**Dependencies:** Qt6 (Core, Gui, Qml, Quick, QuickControls2, Sql, Svg, Multimedia, Bluetooth, Test). On macOS with Homebrew Qt: ensure `Qt6_DIR` or `CMAKE_PREFIX_PATH` points to the Qt installation if CMake cannot find it automatically. **Optional:** ONNX Runtime (detection plugin skipped at configure when absent) and llama.cpp via FetchContent (`-DAI_ANALYST=OFF` to disable). The bare-machine rule is law: a clone with neither must configure, build, and pass 100% of tests.

The legacy qmake files (`*.pro`, `Makefile.*`) remain in the tree but are not the active build system.

## Architecture

Nine CMake targets — full dependency graph, device-stack UML, data flows, ER schema, and threading map in `docs/ARCHITECTURE.md`:

| Target | Directory | Role |
|--------|-----------|------|
| `logger-core` (shared) | `QML_ML_Camera/logger-core/` | CSV logger via `qInstallMessageHandler`; `QLoggingCategory` wrappers (`logInfo()` etc.). |
| `camera-core` (shared) | `QML_ML_Camera/camera-core/` | SQLite persistence (`DatabaseManager` + DAOs), `QAbstractListModel` bridges to QML, `StorageLocations`. Really "data-core"; name kept deliberately. |
| `spectro-core` (shared) | `QML_ML_Camera/spectro-core/` | Analysis math, sensor-neutral device stack (transports, sans-IO codecs, devices), BLE, peak matching. |
| `spectro-sim` (shared) | `QML_ML_Camera/spectro-sim/` | Physics synthesis, instrument-noise model, simulated devices, byte-level bridge impersonation, fault injection. |
| `processor-api` (interface) | `QML_ML_Camera/processor-api/` | `FrameProcessor` v1.1 plugin interface + `Detection` types. |
| `plugins/grayscale`, `plugins/objectdetect` (modules) | `QML_ML_Camera/plugins/` | Runtime-loaded frame processors; objectdetect needs ONNX Runtime and loads from `app/vision/`. |
| `ai-core` (static, optional) | `QML_ML_Camera/ai-core/` | `AiAnalyst` over llama.cpp (pinned tag b6100). |
| `QML_ML_Camera_App` (executable) | `QML_ML_Camera/app/` | Services (`CameraService`, `SpectrometerService`, `GeigerService`, `ReportService`), QML shell, pipeline composition in `main.cpp`. |

Key conventions (enforced, with rationale in `SPECTRO_TRICORDER_EXECPLAN.md`'s Decision Log):

- **Thin views:** all logic in C++; QML is layout, styling, bindings, and invokable calls only.
- **Sans-IO codecs:** protocol parsers never touch I/O; transports move bytes and know nothing of their meaning.
- **Design tokens:** every color/size/font in QML comes from `Theme.qml` (`pragma Singleton`). Yellow `#FFE100` is the only accent; red is reserved for record/alert/destructive.
- **Storage:** everything lives under `QStandardPaths::AppDataLocation` via `StorageLocations` — database at `StorageLocations::databasePath()`, plus `pictures/`, `recordings/`, `exports/`, `models/`, `logs/`. Nothing is written to the current working directory.
- New QML files must be added to the `qt_add_resources` FILES list in `QML_ML_Camera/app/CMakeLists.txt` or `qrc:/` loads fail at runtime.
- New DB tables: `CREATE TABLE IF NOT EXISTS` in the DAO, wire into `DatabaseManager`, add the explicit cascade in the owning DAO's remove path, plus a `:memory:` round-trip + cascade test. Old on-disk databases must upgrade in place.

## ExecPlans (PLANS.md)

Feature work in this repo uses "ExecPlan" design documents defined by `PLANS.md`. When implementing a non-trivial feature, read `PLANS.md` in full and author a self-contained ExecPlan markdown file before writing code. ExecPlans are living documents: update `Progress`, `Decision Log`, and `Surprises & Discoveries` as work proceeds. The active plan is `SPECTRO_TRICORDER_EXECPLAN.md`.

## Logging

Logs are written to `<AppDataLocation>/logs/SolidBroccoli_Log_<date>.csv` via `StorageLocations::logsDir()` — on macOS that is `~/Library/Application Support/SolidBroccoli/SolidBroccoli/logs/`. Use `logInfo()`, `logWarning()`, `logCritical()` (from `loggingcategories.h`) with `qDebug()`/`qWarning()` — do not use plain `qDebug()` without a category.
