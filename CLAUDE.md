# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**solid-broccoli** is a Qt6/QML tricorder-style multi-sensor instrument: live spectroscopy (simulated instruments + BLE bridge path), a Geiger-counter modality, a SQLite lab notebook (sessions, captures, measurements, reports), a documentation camera with photo/video albums and a spectrum-overlay recorder, an optional ONNX object-detection plugin, and an optional on-device AI report generator (llama.cpp + Gemma). The full architecture is documented in `docs/ARCHITECTURE.md` (canonical, Mermaid) — read it before structural changes; `docs/USER_MANUAL.md` describes behavior from the user's side.

## Current development status (2026-07-05 — authoritative detail lives in the active plan's Progress section)

Active branch: `dev_modern_update`. **Active plan: `SPECTRO_FIELD_READINESS_EXECPLAN.md`.** Its predecessor `SPECTRO_TRICORDER_EXECPLAN.md` is closed as complete (Revision 5): the desktop product is done — Milestones 1–10 and 12–14 shipped, 21 test executables green, bare-machine builds verified. Everything still open lives in the field-readiness plan, and every one of its milestones is gated:

- **Milestone 1** — vision model field acceptance; gate: a downloaded YOLO `.onnx` (Settings → VISION → Import).
- **Milestone 2** — AI analyst field acceptance; gate: a downloaded Gemma `.gguf` (Settings → AI → Choose).
- **Milestone 3** — live hardware over BLE; gate: an ESP32 bridge with AS7265x and/or a Geiger tube on the desk (wire contract embedded in the plan and in `BridgeContract.h`).
- **Milestone 4** — Android tablet bring-up; gate: Qt for Android kit + physical tablet. Do NOT start without both.
- **Hardening backlog** — ten recorded candidates (app-core library, CI, QML boot smoke test, in-app Help, parser fuzzing, …), *recorded not scheduled*: take one only when the maintainer asks.

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

**Dependencies:** Qt6 (Core, Gui, Qml, Quick, QuickControls2, Sql, Svg, Multimedia, Bluetooth, Test). On macOS with Homebrew Qt: ensure `Qt6_DIR` or `CMAKE_PREFIX_PATH` points to the Qt installation if CMake cannot find it automatically. **Optional:** ONNX Runtime (detection plugin skipped at configure when absent) and llama.cpp via FetchContent, pinned tag b6100 (`-DAI_ANALYST=OFF` to disable). The bare-machine rule is law: a clone with neither must configure, build, and pass 100% of tests.

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
- **Docs stay true:** structural changes (targets, tables, threads, pipelines, public device-stack API) must update `docs/ARCHITECTURE.md` and its `docs/architecture.html` mirror in the same change — a wrong diagram is worse than none.

## Testing

- Full suite: `ctest --test-dir QML_ML_Camera/build --output-on-failure`; one test: append `-R tst_geiger`. Tests live in `QML_ML_Camera/tests/`, registered via the `add_camera_test()` helper in `tests/CMakeLists.txt`.
- App-layer classes (services, models, overlay, context builder) are compiled source-by-source into their tests (see `tst_spectrometerservice` for the pattern) because the app layer has no library yet — backlog item 1 (`app-core`) will change this.
- Philosophy: analysis math and codecs are validated against **simulator ground truth** (the simulator knows what it injected — peak positions, Poisson means, frame bytes). Tests that need real model files `QSKIP` when absent (`SOLIDBROCCOLI_TEST_GGUF` gates the AI generation smoke test); the suite must be 100% green on a bare machine.
- Proving the optional-dependency axes needs a scratch build: hide Homebrew with `-DCMAKE_IGNORE_PATH="/opt/homebrew/include;/opt/homebrew/lib"` (setting the find variables to `NOTFOUND` does **not** work — CMake re-searches), and disable AI with `-DAI_ANALYST=OFF`.
- Run qmllint after any QML change. The current baseline is ~387 warnings in three buckets. **(1) Structural, not actionable today (~358):** context properties (`albumModel`, `reportService`, `owningStack`, …) and the C++-registered `SpectrumView`/`StripChartView` types are invisible to qmllint, so every use is flagged `[unqualified]`/`[import]`/`[unresolved-type]` (the recorded fix is exposing the app as a QML module and migrating context properties to registered singletons — hardening backlog). **(2) Real, pre-existing, ~29 `[Quick.layout-positioning]`:** setting `width`/`height` on a Layout-managed item (Qt calls it undefined behavior — use `Layout.preferredWidth`/`implicitWidth`). These span AnalysisPanel/CameraPage/GeigerPage/LivePage/SessionDetailPage and want a *verified* cleanup pass (fixing them can shift layouts), not a blind sweep — recorded backlog. **(3) Everything else is a real finding to fix now:** `[unused-imports]`, `[missing-property]` (e.g. a real `Qt.AlignTop` typo), `[confusing-expression-statement]` (a block binding missing `return`), duplicate/invalid signal overrides. The rule: **introduce no new bucket-2 or bucket-3 warnings**, and drive the existing bucket-2 count down deliberately.

## ExecPlans (PLANS.md)

Feature work in this repo uses "ExecPlan" design documents defined by `PLANS.md`. When implementing a non-trivial feature, read `PLANS.md` in full and author a self-contained ExecPlan markdown file before writing code. ExecPlans are living documents: update `Progress`, `Decision Log`, and `Surprises & Discoveries` at every stopping point, and commit at milestone boundaries with the milestone name in the message. The active plan is `SPECTRO_FIELD_READINESS_EXECPLAN.md`; `SPECTRO_TRICORDER_EXECPLAN.md` is the closed desktop-scope record (rationale in its Decision Log, hard-won lessons in its Surprises & Discoveries).

Four project subagents are defined in `.claude/agents/` for milestone work: `spectro-architect` (refines the next milestone against the tree before coding), `spectro-builder` (implements), `spectro-scientist` (tests and scientific validation), `spectro-reviewer` (reviews the milestone diff before commit). They defer to the **active** plan (`SPECTRO_FIELD_READINESS_EXECPLAN.md`) as the single source of truth; the closed tricorder plan remains the reference for settled rationale (Decision Log) and hard-won lessons (Surprises & Discoveries).

## Sibling projects (same sandbox, same maintainer)

- `../Coffee_Dispenser` — Qt6/QML coffee-machine HMI being modernized per its own `COFFEE_DISPENSER_MODERNIZATION_EXECPLAN.md` (authored 2026-07-05, M0 not started). It deliberately reuses the patterns proven here — CMake target layering, sans-IO codecs, simulator-first with fault injection, thin views, design tokens, gated hardware milestones — and has its own `CLAUDE.md` and a `dispenser-builder` agent. The ExecPlan discipline applies there even though `PLANS.md` lives only in this repo.
- `../../llm/dobromir` — pre-alpha LLM/VLM project (photo of a problem → wordless IKEA-style repair schematic). Not Qt; different laws (`snake_case` naming, no-training-before-eval, learning milestones the maintainer implements *by hand*). See its `CLAUDE.md` and `ROADMAP.md` before doing anything there.

## Gotchas (each cost real time once; details in the plan's Surprises log)

- A **partial Homebrew Qt upgrade** crashes the app at launch (`dyld: Symbol not found … QtPrivate_6_11_…`). Fix: `brew upgrade qt` so all Qt formulae match; consider `brew pin qt qtbase qtdeclarative qtmultimedia qtconnectivity qtsvg`.
- Camera activation must go through the `QCameraPermission` check/request in `CameraService::setActive` — since Qt 6.5, `QCamera::setActive(true)` alone never raises the macOS permission dialog. If a machine auto-denied once: `tccutil reset Camera com.solidbroccoli.qmlmlcamera`.
- Qt Quick Controls is pinned to the **"Basic" style** in `main.cpp`; the native macOS style silently ignores `contentItem`/`background` customization. Don't remove the pin.
- Never bind `font.family` to a possibly-empty string — Qt resolves `""` to the alphabetically first family (Apple Color Emoji on macOS). Use `Theme.fontName`.
- `FrameProcessor` is a **binary plugin interface**: adding a virtual breaks ABI — bump the IID with it (currently `broccoli.FrameProcessor/1.1`) so stale plugins fail loudly at load.
- BLE scanning runs only on explicit `refreshDevices()` (Settings → Refresh) — never in constructors; unit tests instantiate services freely and must stay radio- and permission-prompt-free.

## Logging

Logs are written to `<AppDataLocation>/logs/SolidBroccoli_Log_<date>.csv` via `StorageLocations::logsDir()` — on macOS that is `~/Library/Application Support/SolidBroccoli/SolidBroccoli/logs/`. Use `logInfo()`, `logWarning()`, `logCritical()` (from `loggingcategories.h`) with `qDebug()`/`qWarning()` — do not use plain `qDebug()` without a category.
