# Take the solid-broccoli tricorder to the field: model verification, real hardware over BLE, Android tablet bring-up, and engineering hardening

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds. It follows the repository guidance in `PLANS.md` (repository root) and must be maintained in accordance with it.

This plan is the successor to `SPECTRO_TRICORDER_EXECPLAN.md` (repository root, checked in — incorporated by reference for history and rationale). That plan built the product and is closed as complete; **this plan carries everything that could not be finished on a bare development Mac**: acceptances gated on downloadable model weights, on physical sensor hardware, and on an Android toolchain, plus the recorded engineering-hardening backlog. Nothing here blocks anything else here; milestones unlock independently as their gates open.

## Purpose / Big Picture

Today the repository builds a complete tricorder-style instrument that is fully verified *against simulation*: spectra come from physics-based simulated devices (including a byte-level simulated BLE bridge), radiation from a simulated Poisson tube, object detection and AI reports are implemented and tested but idle until a model file is supplied. After this plan, the same app is verified *against reality*: a YOLO model draws live boxes on the camera preview; a Gemma model writes grounded session reports; a real ESP32 bridge streams real AS7265x spectra and real Geiger counts over Bluetooth Low Energy; and the whole instrument runs on a physical Android tablet. A hardening backlog (CI, an app-level library, a QML smoke test, fuzzing, UX unification) is recorded here and promoted to milestones on explicit request.

How to see it working at the end: on the tablet (or the Mac), the Live tab streams a real spectrum from the bridge on the desk; the Camera tab labels the objects in front of it; a session's "Generate AI report" writes prose whose identification table was computed deterministically; and `ctest` still passes 100% on a machine with none of the optional pieces installed.

**The gating rule (inherited, non-negotiable):** a milestone whose gate (hardware, model file, toolchain) is absent is marked blocked in `Progress` and skipped without failing anything — the suite stays green on a bare machine. Never bypass a gate by faking its acceptance.

## Progress

- [x] (2026-07-05) Plan authored: unfinished scope extracted from `SPECTRO_TRICORDER_EXECPLAN.md` Revision 5 (its Milestone 11, the M9/M10 hardware acceptances, the M12/M13 model acceptances, the Revision 4 improvement backlog, and the future-extensions list). Numbering restarts here; the mapping is recorded in the Decision Log.
- [ ] Milestone 1 (gated: YOLO `.onnx` download): vision model field acceptance.
- [ ] Milestone 2 (gated: Gemma `.gguf` download): AI analyst field acceptance.
- [ ] Milestone 3 (gated: ESP32 bridge + AS7265x and/or Geiger tube on the desk): live hardware over BLE.
- [ ] Milestone 4 (gated: Qt for Android kit + physical tablet): Android tablet bring-up.

## Surprises & Discoveries

- (None yet — to be filled during implementation.)

## Decision Log

- Decision: Unfinished work moves out of the tricorder plan into this successor, and the tricorder plan is closed as complete. Milestone numbering restarts at 1 here; the mapping is: tricorder Milestone 11 (Android) → Milestone 4 here; tricorder Milestone 9/10 gated hardware acceptances → Milestone 3 here; tricorder Milestone 12/13 gated model acceptances → Milestones 1–2 here; the Revision 4 improvement backlog and the future-extensions list move verbatim (cross-references adjusted).
  Rationale: Maintainer request (2026-07-05) — a plan that is finished should read as finished; one unchecked box and four "gated" notes made the completed plan look perpetually open. Living work needs a living home. PLANS.md self-containment is preserved by embedding every actionable detail here rather than pointing back.
  Date/Author: 2026-07-05 / Claude + maintainer
- Decision: Milestone order is models (1–2) before hardware (3) before Android (4).
  Rationale: The model gates open with a download — the maintainer can unlock them today; hardware needs devices on the desk; Android needs a toolchain and re-runs Milestones 1–3 on the device, so it goes last.
  Date/Author: 2026-07-05 / Claude
- Decision: The hardening backlog stays a backlog (recorded, not scheduled) rather than becoming milestones 5–14.
  Rationale: Each item becomes a milestone only when the maintainer takes it; pre-scheduling ten refactors would recreate the perpetually-open-plan problem this split exists to solve.
  Date/Author: 2026-07-05 / Claude

## Outcomes & Retrospective

- (2026-07-05) Plan authored; no milestone started — all four are gated and their gates are currently closed.

## Context and Orientation

The repository root contains `QML_ML_Camera/`, built with CMake into nine targets. The ones this plan touches: `spectro-core` (shared library: analysis math, the sensor device stack — byte transports, sans-IO protocol codecs, devices — and `BleTransport`), `spectro-sim` (shared library: physics-based simulated devices and `SimulatedBridgeTransport`, a byte-level impersonation of the real bridge), `camera-core` (SQLite persistence and `StorageLocations`, which puts all app data under the platform `AppDataLocation`), `plugins/objectdetect` (an optional plugin built only when ONNX Runtime is found; loaded at runtime from `app/vision/`), `ai-core` (an optional static library wrapping llama.cpp, pinned tag `b6100`, built unless `-DAI_ANALYST=OFF`), and the executable `QML_ML_Camera_App` in `QML_ML_Camera/app/` (services, QML shell). Architecture diagrams live in `docs/ARCHITECTURE.md`; user-facing behavior in `docs/USER_MANUAL.md`.

Terms used below. A "sans-IO codec" is a parser class that translates bytes into readings and commands into bytes but never touches a socket or radio itself — transports move the bytes. "GATT" is the Bluetooth Low Energy attribute protocol: a peripheral offers "services" (grouped by UUID) containing "characteristics" you can write to or subscribe to ("notifications"). An "ONNX" file is a portable neural-network graph executed here by ONNX Runtime; "YOLO" is a family of object-detection networks. A "GGUF" file is a quantized large-language-model container executed by llama.cpp. "Gated" means: requires something not in the repository (hardware, a downloaded file, a toolchain); the gate's absence must never fail a build or test.

Build, run, test (from the repository root; Qt 6.11 via Homebrew at `/opt/homebrew/opt/qt` on the dev Mac):

    cmake -S QML_ML_Camera -B QML_ML_Camera/build
    cmake --build QML_ML_Camera/build -j
    ctest --test-dir QML_ML_Camera/build --output-on-failure
    /opt/homebrew/opt/qt/bin/qmllint QML_ML_Camera/app/*.qml
    ./QML_ML_Camera/build/app/QML_ML_Camera_App.app/Contents/MacOS/QML_ML_Camera_App

The suite is 21 test executables and must stay 100% green throughout, including on a machine with no ONNX Runtime and no llama.cpp (`-DCMAKE_IGNORE_PATH="/opt/homebrew/include;/opt/homebrew/lib"` simulates the former on the dev Mac — note that overriding CMake find variables to `NOTFOUND` does *not* work, CMake re-searches; `-DAI_ANALYST=OFF` disables the latter).

**The bridge wire contract** (authoritative header: `QML_ML_Camera/spectro-core/BridgeContract.h`; the firmware for Milestone 3 must implement it). The peripheral advertises service UUID `5B0C0001-8E2B-4D8B-9C60-0A5B3D1EAD01` with two characteristics: data `5B0C0002-8E2B-4D8B-9C60-0A5B3D1EAD01` (notify) and control `5B0C0003-8E2B-4D8B-9C60-0A5B3D1EAD01` (write). Every data frame is: u32 little-endian payload-plus-header length, then magic `EC 5B` (0x5BEC), version `01`, a frame-type byte, then the payload. Spectrum frames (type `01`): u8 channel count (`12` hex = 18), u32 timestamp in ms, then 18 × float32 little-endian counts — the channel wavelengths are fixed by the AS7265x hardware (410, 435, 460, 485, 510, 535, 560, 585, 610, 645, 680, 705, 730, 760, 810, 860, 900, 940 nm; the codec owns this table). Geiger frames (type `02`): u32 timestamp, float32 counts-per-second. Commands written to the control characteristic: `01` start streaming, `02` stop, `03` + u16 LE integration-time ms + u8 averaging count. Notifications may be chunked to any MTU (the parser reassembles; 20-byte chunks are the worst case and what the simulator emits); the parser resynchronizes byte-wise after corruption by trusting a frame only when length, magic, and version agree at the same offset. `SimulatedBridgeTransport` in `spectro-sim` plus the byte-level tests `tst_as7265xcodec` and `tst_geiger` are the executable specification: firmware that satisfies them satisfies the app.

Where user files live (macOS): everything under `~/Library/Application Support/SolidBroccoli/SolidBroccoli/` — the SQLite database, `logs/` (CSV, one per day — read these first when anything misbehaves), `exports/`, `models/` (imported ONNX files; GGUF files are referenced in place, never copied).

## Plan of Work

### Milestone 1 — Vision model field acceptance (gated: a YOLO `.onnx` file)

Everything is implemented and unit-tested; what has never happened is a real model drawing real boxes. Obtain a YOLO11n (or YOLOv8n) ONNX file, either exported locally —

    brew install python@3.12
    /opt/homebrew/opt/python@3.12/bin/python3.12 -m venv ~/yolo-venv
    ~/yolo-venv/bin/pip install ultralytics
    ~/yolo-venv/bin/yolo export model=yolo11n.pt format=onnx

— or downloaded pre-exported (~10 MB). The app must have been configured with ONNX Runtime present (`brew install onnxruntime`; the configure log then builds `objectdetect-processor` and the app log shows "Vision: detection plugin loaded"). In the app: Settings → VISION → Import… (the file is copied into the model store with its SHA-256 recorded), toggle Object detection on, open the Camera tab.

Acceptance (from the tricorder plan's Milestone 12, verbatim): labeled yellow boxes with confidence chips on the preview at ≥15 FPS with detections updating ≥5 Hz at the default stride 3; the chip row on the Camera page lists current objects; a recording made with detection and the spectrum overlay on plays back with boxes burned in; toggling detection off restores the direct path. Any defect found is fixed under this milestone and recorded in `Surprises & Discoveries`.

### Milestone 2 — AI analyst field acceptance (gated: a Gemma `.gguf` file)

Same shape: implemented, tested model-free, never run against a real model. Download **Gemma 3 4B instruct QAT Q4_0** (~2.4 GB file, ~3 GB working RAM; repository `ggml-org/gemma-3-4b-it-qat-GGUF` on Hugging Face) or, on 8 GB machines, **Gemma 3 1B** (~0.7 GB). Gemma's license terms accompany the download; the app shows the notice in Settings → AI. Choose the file via Settings → AI → Choose… (referenced in place). Open a session containing mercury-lamp captures (record one on the Live tab against "Simulated UV-Vis (mercury lamp)" if none exists) and press "Generate AI report".

Acceptance (from the tricorder plan's Milestone 13, verbatim): tokens stream visibly into the viewer; the finished report names the mercury lamp via the embedded deterministic match table, states the integration settings correctly, ends with the fixed caveat footer ("AI-generated interpretation — verify against calibration and reference standards."), persists across an app restart, and exports as Markdown next to the CSV/JSON exports. Additionally run the gated generation smoke test once:

    SOLIDBROCCOLI_TEST_GGUF=/path/to/model.gguf ctest --test-dir QML_ML_Camera/build -R tst_aianalyst --output-on-failure

and record its runtime in `Artifacts and Notes`.

### Milestone 3 — Live hardware over BLE: the ESP32 bridge (gated: hardware on the desk)

The wire contract above is frozen and simulator-proven; this milestone is first contact with physical radio and physical sensors. The bridge is an ESP32 (or any BLE-capable microcontroller) exposing the service/characteristics/frames exactly as specified, with an AS7265x spectral sensor and/or a Geiger tube behind it. Firmware development is out of scope for this repository, but the contract section of this plan plus `BridgeContract.h` plus the simulator are its complete specification — develop firmware against `tst_as7265xcodec`'s hand-built frames if in doubt.

In the app: Settings → SPECTROMETER → Refresh runs a 5-second BLE scan (first run triggers the macOS Bluetooth permission prompt); devices advertising the service UUID appear in the device list alongside the simulators. Connect, then Start on the Live tab.

Acceptance, spectrometer path: the real bridge appears after Refresh; Start streams 18-channel spectra rendered live; changing integration time/averaging in Settings reaches the firmware (command `03` observed); Dark/Reference capture and absorbance work against a real lamp; captures save into sessions and export. Acceptance, Geiger path: connecting the Geiger device streams type-`02` frames; the RADIATION card wobbles with real counts; CPM/dose track a check source moved closer and farther; measurements save and the windowed CSV exports. Robustness while connected: walking out of radio range mid-stream must produce a clean disconnect (device state, no crash, log entry) and reconnecting must resume — the fault-injection tests promise this; reality confirms it. Capture at least one real byte trace with `TraceRecorder` (spectro-sim) and check the replay parses identically; store the observation in `Surprises & Discoveries`.

### Milestone 4 — Android tablet bring-up (gated: Qt for Android kit + physical tablet)

Do not start this milestone unless a Qt for Android kit (Qt 6.11 for Android, JDK, Android SDK/NDK) and a physical Android tablet are present; otherwise it stays blocked in `Progress`. This is the tricorder plan's Milestone 11, unstarted, carried here whole.

Scope when unblocked: a CMake preset (or documented `qt-cmake` invocation) for the Android kit; `AndroidManifest.xml` with BLE permissions (`BLUETOOTH_SCAN`/`BLUETOOTH_CONNECT` with `neverForLocation`) and camera permission; runtime permission requests via `QBluetoothPermission`/`QCameraPermission` in the services before first use (the camera half already exists in `CameraService::setActive` from desktop work; the Bluetooth half must be added where `refreshDevices()` starts the scan); a layout pass verifying the ≥900 px side-panel mode on the tablet and touch pinch-zoom in `SpectrumView` and the strip chart; and re-running Milestones 1–3 of this plan on the tablet. Two knowns to plan around, recorded when the tricorder plan was written: runtime plugin loading is not available the desktop way on mobile, so the grayscale/objectdetect plugins move to static linking (`Q_IMPORT_PLUGIN`) behind the same one-toggle-one-owner composition in `main.cpp`; and ONNX Runtime on Android is a prebuilt AAR/library with the NNAPI/XNNPACK execution providers, not a Homebrew path — the plugin's `find_path`/`find_library` guard needs an Android branch. One risk to resolve early: Android file pickers return content URIs, not filesystem paths, and llama.cpp needs a real path — the GGUF "referenced in place" policy may need a copy-into-app-storage fallback on Android; decide and record in the Decision Log when the gate opens. iOS (and the USB-probe bridge it requires) remains a separate future ExecPlan.

Acceptance: the app installs and launches on the tablet; the Live tab streams the mercury-lamp simulator at interactive frame rates; permission prompts appear at first camera/Bluetooth use and denial degrades gracefully; the six-tab layout and side panel render correctly in both orientations; and whichever of Milestones 1–3's gates are open pass their acceptances on the device.

### Hardening backlog (recorded, not scheduled — promoted to milestones on explicit request)

Candidates in recommended order, moved from the tricorder plan's Revision 4 reflection. Each becomes a milestone only when explicitly taken; none is implied by this plan's existence.

1. **`app-core` static library** — move the app-layer services and models that tests currently recompile source-by-source (`SpectrometerService`, `GeigerService`, `SpectrumOverlayProcessor`, `PluginManager`, `DetectionModel`, `ReportContextBuilder`, `PeakListModel`, `CaptureCoordinator`) into a static library linked by both the app and the tests. Removes six per-test recompiles, guarantees the tests exercise the shipped object code, and simplifies `QML_ML_Camera/tests/CMakeLists.txt`. Mechanical, low risk.
2. **CI workflow** — GitHub Actions on a macOS runner: configure, build, ctest, qmllint; plus a second job proving the bare-machine rule continuously (`CMAKE_IGNORE_PATH` hiding Homebrew, `-DAI_ANALYST=OFF`). Turns the founding rule from discipline into machinery.
3. **QML boot smoke test** — a ctest that runs the real `QQmlApplicationEngine` with all context properties under `QT_QPA_PLATFORM=offscreen` and asserts `main.qml` loads with zero warnings. Covers the one regression class the unit tests cannot see — broken bindings, missing context properties, qrc omissions; exactly the `autoOrientation` failure mode in the tricorder plan's Surprises log.
4. **In-app Help** — bundle `docs/USER_MANUAL.md` into resources and render it in a `TextArea` (`TextEdit.MarkdownText`) behind a Settings row. One source with the repo manual; a field instrument must not require a laptop to read its own manual.
5. **`BridgeFrameParser` fuzz/property test** — seeded random byte streams and systematic single-byte corruptions; invariants: never crashes, every intact frame embedded in noise is eventually recovered, resync cost bounded. Pure code, cheap, and the right insurance before Milestone 3's first contact with real firmware.
6. **Notification unification** — one C++ `AppNotifier` (info/warning/error) plus one toast overlay in `main.qml`, replacing the per-page error toasts that grew page by page. Consistent UX, less QML duplication.
7. **Context properties → registered singletons** — migrate `main.cpp`'s `setContextProperty` surface to `qmlRegisterSingletonInstance` so qmllint can see the names (unqualified lookups are invisible to tooling today) and editors can complete them. Mechanical but touches every page; take after item 3 so the smoke test guards the migration.
8. **`DatabaseManager` encapsulation** — DAOs are public members reached into as `db.m_sessionDao…`; introduce accessors and make the members private. Pairs naturally with item 1.
9. **Sanitizer preset** — an ASan/UBSan CMake configuration run over ctest locally (and as a third CI job); codecs and byte parsers are where undefined behavior hides.
10. **Spectrum cursor readout** — tap/hover on `SpectrumView` shows the nearest point's λ/value (and, for stored captures, the `PeakMatcher` candidate). The most natural touch interaction on the most-touched surface; hit-testing in C++ keeps the view thin.

The observations behind the list, preserved so the reasoning survives: *UX* — empty states rely on Settings knowledge (no device connected, no model installed → features look absent instead of guiding); errors surface per-page with per-page styles; the manual exists only outside the app; the plot is display-only. *Architecture* — the app layer is the only layer without a library boundary; tooling cannot see context properties; `DatabaseManager` leaks its internals. *Testing* — coverage is deep on pure math and byte paths, absent on QML wiring; the founding build rules are enforced by habit, not automation.

### Future extensions (recorded, not scheduled)

A-scan ultrasound (echo amplitude vs. time-of-flight, as in thickness gauges and rangefinders) is structurally a spectrum — two equal-length arrays with a different x-axis — and the hooks it needs already exist in the tree, built by the tricorder plan: the sensor-neutral device stack in `spectro-core`, `SpectrumView`'s configurable x-axis, and the frame-type byte in the bridge contract (a future type `03` would carry echo waveforms). When it becomes real, it is an additive milestone shaped like the Geiger one. Imaging (B-mode) ultrasound remains permanently out of scope: megabyte-per-second data rates exceed BLE entirely, and commercial imaging probes ship proprietary mobile SDKs — pursuing it would be a separate product decision and ExecPlan, not a sensor addition. Also recorded from the tricorder plan: Gemma 3 4B is multimodal, and attaching a captured still via llama.cpp's mmproj path could let reports describe the scene directly — a desktop-only experiment first, if ever.

## Concrete Steps

All commands run from the repository root. The standing loop after any change:

    cmake -S QML_ML_Camera -B QML_ML_Camera/build
    cmake --build QML_ML_Camera/build -j
    ctest --test-dir QML_ML_Camera/build --output-on-failure
    /opt/homebrew/opt/qt/bin/qmllint QML_ML_Camera/app/*.qml
    ./QML_ML_Camera/build/app/QML_ML_Camera_App.app/Contents/MacOS/QML_ML_Camera_App

Model acquisition commands are inside Milestones 1–2. Update this plan's `Progress`, `Surprises & Discoveries`, and `Decision Log` at every stopping point; commit at milestone boundaries with the milestone name in the message. The four project subagents in `.claude/agents/` (`spectro-architect`, `spectro-builder`, `spectro-scientist`, `spectro-reviewer`) apply to this plan exactly as they did to its predecessor; this plan is the source of truth they defer to.

## Validation and Acceptance

Per milestone, as written in each. Whole-plan end state: with all four gates opened at least once, the instrument has been observed doing real work — real boxes from a real camera scene, a real grounded report from a real model, real spectra and real counts from a real bridge, all of it on a physical tablet — while a bare clone with no models, no ONNX Runtime, no llama.cpp, and no hardware still configures, builds, and passes the entire suite. The hardening backlog has either been promoted through explicit maintainer requests (each then carrying its own acceptance) or remains recorded and untouched.

## Idempotence and Recovery

Milestones 1–2 are pure verification plus fallout fixes: re-runnable at will; a failed acceptance leaves the tree untouched apart from recorded findings and their fixes. Milestone 3 changes nothing in the repository unless defects surface; byte traces recorded during it are new files. Milestone 4 is additive (preset, manifest, permission calls, static-plugin plumbing) and revertable via git; it must not disturb the desktop build — configure both kits side by side before declaring it done. If any milestone is interrupted, split its `Progress` entry into done/remaining before stopping.

## Artifacts and Notes

Expected bridge data frames (Milestone 3, little-endian throughout): a spectrum frame begins `4F 00 00 00 EC 5B 01 01 12`, i.e. length 0x4F = 79 bytes of header-after-length plus payload (magic+version+type+count+timestamp+18 floats), then the u32 timestamp and 72 bytes of counts; a Geiger frame is `0C 00 00 00 EC 5B 01 02` + u32 timestamp + float32 CPS. Commands: `01`, `02`, `03 64 00 04` (start; stop; params 100 ms × 4).

Log lines that prove wiring during the milestones (CSV logs under `~/Library/Application Support/SolidBroccoli/SolidBroccoli/logs/`): "Vision: detection plugin loaded" (Milestone 1 precondition), "SpectrometerService: 3 device(s) available." growing to 4+ after a successful scan (Milestone 3), and the report-persistence lines from `ReportService` (Milestone 2).

## Interfaces and Dependencies

Nothing in Milestones 1–3 adds a dependency: ONNX Runtime and llama.cpp integration exist and stay build-time optional; the models are user-supplied files. Milestone 4 adds the Qt for Android kit (Qt 6.11 for Android, JDK, Android SDK/NDK) on the development machine only, an Android-capable ONNX Runtime distribution for the plugin if detection is wanted on the tablet, and no new runtime services. The wire contract in `QML_ML_Camera/spectro-core/BridgeContract.h` is frozen: firmware conforms to it, not the other way around — extending it (new frame types) bumps the version byte and gets its own plan revision. The `FrameProcessor` plugin interface stays at IID `broccoli.FrameProcessor/1.1`; static linking on Android must not fork it.
