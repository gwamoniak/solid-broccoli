# solid-broccoli

A tricorder-style multi-sensor instrument for tablets and desktops, built with
Qt 6/QML — dark instrument UI, one yellow accent, everything developable with
zero hardware thanks to a physics-based simulation library.

**What it does:**

- **Spectroscopy** — live intensity-vs-wavelength plot at interactive frame
  rates, dark/reference calibration, transmittance/absorbance, peak detection
  and labeling, region integration; simulated instruments (mercury lamp,
  Beer-Lambert dye sample) plus a BLE bridge path for real hardware
  (ESP32 + AS7265x).
- **Radiation** — Geiger-counter modality: live dose rate (µSv/h), rolling
  CPM, scrolling strip chart with alert threshold, honest Poisson statistics
  in simulation.
- **Lab notebook** — sessions in SQLite: tagged spectral captures, dose
  measurements, linked videos; CSV/JSON export.
- **Documentation camera** — photos and video with the live spectrum and dose
  burned into recordings; runtime frame-processor plugins.
- **Object detection** *(optional)* — YOLO via ONNX Runtime on the live
  preview, decoupled from the render loop.
- **On-device AI reports** *(optional)* — Gemma via llama.cpp writes session
  summaries grounded in deterministic peak identification; every report
  stores its exact input facts for audit. No cloud, no telemetry.

## Documentation

| Document | Contents |
|---|---|
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | Architecture atlas: build targets, device-stack UML, data flows, ER schema, threading (Mermaid, canonical) |
| [docs/architecture.html](docs/architecture.html) | The same diagrams as a styled standalone page |
| [docs/USER_MANUAL.md](docs/USER_MANUAL.md) | Operating manual: workflows, settings, troubleshooting, data locations |
| [SPECTRO_FIELD_READINESS_EXECPLAN.md](SPECTRO_FIELD_READINESS_EXECPLAN.md) | The active plan: model/hardware field verification, Android bring-up, hardening backlog |
| [SPECTRO_TRICORDER_EXECPLAN.md](SPECTRO_TRICORDER_EXECPLAN.md) | The closed desktop-scope design record — every decision and its rationale |

## Build

CMake + Qt 6.5 or newer (Core, Gui, Qml, Quick, QuickControls2, Sql, Svg,
Multimedia, Bluetooth, Test). From the repository root:

    cmake -S QML_ML_Camera -B QML_ML_Camera/build
    cmake --build QML_ML_Camera/build -j

Run the app:

    ./QML_ML_Camera/build/app/QML_ML_Camera_App.app/Contents/MacOS/QML_ML_Camera_App

Run the test suite:

    ctest --test-dir QML_ML_Camera/build --output-on-failure

**A bare machine builds green.** ONNX Runtime, llama.cpp, and model weights
are strictly optional:

| Dependency | Enables | Without it |
|---|---|---|
| ONNX Runtime (`brew install onnxruntime`) | object-detection plugin | plugin skipped at configure; all else builds and passes |
| llama.cpp (fetched automatically; disable with `-DAI_ANALYST=OFF`) | AI report generation | reports remain viewable/exportable |
| Model weights (YOLO `.onnx`, Gemma `.gguf`) | the above at runtime | imported/chosen by the user in Settings; never bundled |

## Icons

The toolbar/button icons in `QML_ML_Camera/app/images/svg/` are original,
single-color SVG glyphs authored for this project and released under
[CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/) (public domain).
Each is a monochrome shape recolored at runtime through a control's
`icon.color`, so one source serves any theme and renders crisply at any DPI.
