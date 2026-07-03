# Turn solid-broccoli into a tricorder-style spectroscopy instrument app (C++/Qt, Nikon-inspired dark instrument design)

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds. It follows the repository guidance in `PLANS.md` (repository root) and must be maintained in accordance with it.

## Purpose / Big Picture

Today this repository builds a Qt 6/QML camera-and-gallery application. After this plan, the same application is a professional multi-sensor instrument centered on spectroscopy: it connects to an optical spectrometer (initially high-fidelity simulated devices, then real hardware over Bluetooth Low Energy), shows a live intensity-vs-wavelength plot at interactive frame rates, computes absorbance/transmittance against captured dark and reference spectra, detects and labels peaks, integrates regions, saves tagged captures into sessions stored in SQLite, exports CSV/JSON, and can record a camera video with the live spectrum drawn on top for documentation. The UI is restyled into a dark, matte, Nikon-inspired instrument look: near-black surfaces, a single yellow accent, hairline separators, and monospaced numeric readouts, laid out for tablets (multi-panel on wide screens).

A dedicated simulation library (`spectro-sim`) makes the entire product developable and testable with zero hardware: it synthesizes physically plausible spectra (emission lamps, Beer-Lambert absorbance samples, blackbody sources), pushes them through a configurable instrument-noise model, can impersonate the future Bluetooth bridge at the byte level, injects faults (disconnects, corrupt frames), and records/replays byte traces. Because the simulator knows its own ground truth, the analysis pipeline is validated scientifically: tests assert that peak detection recovers the mercury lines it injected and that absorbance analysis recovers the absorption band depth it configured.

Below its top layer the device stack is sensor-neutral: transports move bytes and codecs parse them, for any instrument. Milestone 10 proves it with a second modality — a Geiger counter with a live dose-rate readout, a scrolling count chart, sessions, export, and video overlay — and the stack is shaped so A-scan ultrasound (echo amplitude vs. time, structurally a spectrum) can follow later without rework. Imaging (B-mode) ultrasound is explicitly out of scope.

How to see it working at the end: build and run the app; the Live tab connects to "Simulated UV-Vis (mercury lamp)"; pressing Start shows a moving yellow trace whose dominant peak reads 546.1 nm; switching the simulated scene to the dye sample and capturing Dark, then Reference, then selecting Absorbance shows a real absorption band whose integral is stable; Save stores the capture into a session visible in the Sessions tab; Export writes a CSV you can open; the Camera tab records video with the live trace overlaid; and connecting the simulated Geiger counter adds a live dose-rate card whose counts wobble the way real radiation does (Poisson statistics).

Everything is C++ and QML. No Rust, no new third-party libraries. QML stays a thin view (layout, styling, bindings, invokable calls); all logic lives in C++ — this is an existing, enforced project convention.

## Progress

- [x] (2026-07-03) Planning: repository inspected, architecture agreed (sans-IO protocol codecs behind a transport abstraction, simulator-first), design direction chosen (Nikon-style dark instrument theme, yellow accent), this ExecPlan authored.
- [x] (2026-07-03) Revision 1: dedicated `spectro-sim` simulation library added as its own milestone (physics synthesis, instrument model, fault injection, bridge loopback, trace record/replay); milestones renumbered; project subagents (architect/builder/reviewer/scientist) created under `.claude/agents/`.
- [x] (2026-07-03) Revision 2: device stack made sensor-neutral (`SensorTransport`, `SensorDevice` base, `SensorReading` variant, frame-type byte in the bridge contract); Geiger counter support added as Milestone 10; A-scan ultrasound recorded as a future extension; B-mode ultrasound declared out of scope; Android renumbered to Milestone 11.
- [x] (2026-07-03 21:46) Milestone 1: Instrument design language — `Theme.qml` rethemed to the dark/yellow token set (all new instrument tokens added); tab-bar icons now tinted via `icon.color`; build + 6/6 tests + qmllint clean; app launches with a clean log (visual check on screen left to the maintainer). Included one out-of-scope-but-blocking fix, see Surprises.
- [x] (2026-07-03 22:05) Milestone 2: `spectro-core` library built and tested — `Spectrum`/`GeigerReading`/`SensorReading` value types, `SpectroAnalysis` (Savitzky-Golay, prominence-based findPeaks, boundary-interpolated integrate, clamped transmittance/absorbance), `SensorDevice`/`SpectrometerDevice`, `SensorTransport`, `ProtocolCodec`, generic `CodecDevice`. Two new test executables (`tst_spectroanalysis`, `tst_codecdevice` with in-test fake transport/codec); suite now 8/8 green. One interface refinement recorded in the Decision Log (typed signals on the device base).
- [x] (2026-07-03 22:24) Milestone 3: `spectro-sim` library built and validated — `SpectrumSynthesizer` (mercury/neon lamps, Planck blackbody, Gaussian absorber, Beer-Lambert), seeded `InstrumentModel` (QE bell, shot/read noise, dark current, hot pixels, saturation, averaging, plus a public `idealSignal()` so tests can compute exact residuals), `SimScene` + `SimulatedSpectrometer` (mercuryLamp/dyeSample named constructors, lamp/concentration controls for dark/ref capture), `ScriptedTransport` + `FaultPolicy`, `TraceRecorder`/`ReplayTransport`. `tst_spectrosim`: 12 sub-tests green — peak detection recovers 546.1 nm and ≥4/5 mercury lines; absorbance recovers 0.8 at 664 nm and halves with half concentration; identical seeds bit-identical; integration scales signal not read noise; saturation clips; averaging 4 ≈ 2× noise reduction; chunked/corrupted transport deterministic; trace record/replay round-trips. Suite 9/9 executables.
- [x] (2026-07-03 22:40) Milestone 4: `SpectrometerService` implemented (device list with both simulated instruments, connect/disconnect, acquisition with auto-connect-on-start, integration/averaging params pushed to the device, mutex-guarded latest spectrum, peak/FPS readouts, full logging) and registered as QML singleton; SettingsPage gained the SPECTROMETER section (tap-to-cycle device row, yellow/grey Connect button, integration slider with monospaced readout, 1/2/4/8 averaging segments). Build + 9/9 tests green; app launches with "SpectrometerService: 2 device(s) available." in the log and no QML errors. Interactive click-through (Connect flips to yellow, log records it) left to the maintainer; the service paths get their unit test in Milestone 6 per plan. Note: `selectNextDevice()` invokable added beyond the planned API so the device-picker row stays logic-free in QML.
- [x] (2026-07-03 22:55) Milestone 5: Live tab shipped — `SpectrumView` QQuickItem (scene-graph grid/axes/polylines, wheel + drag + pinch + native-gesture zoom/pan, adaptive tick model, smoothed y-autoscale, generic x-axis, Theme colors passed as properties), service extended with mode (raw/T/A guarded on calibration), hold, scene-driven dark/reference capture (lamp-off / blank-swap on the simulator, snapshot on hardware), calibration reset on device switch; `LivePage` (readout strip with monospaced PEAK λ/PEAK/INTEGRATION/FPS + live dot, plot with QML tick labels, RAW/TRANS/ABS segments with hint, yellow-ring Start/Stop, Dark/Ref/Hold with captured dots, error toast); six-tab shell (Live default, Sessions placeholder) with two new thin-line SVGs. Build + 9/9 tests + lint clean; verified live: Start auto-connected the mercury lamp and streamed at 100 ms until Stop (CSV log evidence). Two platform findings fixed, see Surprises.
- [ ] Milestone 6: Analysis — smoothing, absorbance/transmittance modes, peak markers, integration region, multi-trace overlays.
- [ ] Milestone 7: Sessions & export — SQLite schema + DAOs + models, Sessions tab, tagging, reload overlays, CSV/JSON export.
- [ ] Milestone 8: Video documentation — spectrum overlay on recorded camera video, video linked to session.
- [ ] Milestone 9: BLE transport + AS7265x codec — verified end-to-end against the `spectro-sim` bridge loopback; live-hardware acceptance explicitly gated on a device being available.
- [ ] Milestone 10: Geiger counter — second sensor modality end-to-end against a simulated Poisson tube (dose readout, strip chart, sessions, export, overlay); real-tube acceptance gated.
- [ ] Milestone 11 (gated): Android tablet bring-up — requires Qt for Android kit + device; do not start without them.

## Surprises & Discoveries

- Observation: The iOS-redesign shell is already in the working tree: `main.qml` is a bottom `TabBar` + one `StackView` per tab, `NavPage.qml` provides the per-page navigation bar, and `Theme.qml` is a `pragma Singleton` token file registered in `QML_ML_Camera/app/qmldir` (alongside the legacy `Style` shim). This plan builds on that shell; only the palette and the tab set change.
  Evidence: `QML_ML_Camera/app/main.qml` (StackLayout of four StackViews, footer TabBar), `QML_ML_Camera/app/qmldir` (`singleton Theme 1.0 Theme.qml`).
- Observation (M1, 2026-07-03): The app in the working tree did not launch at all before Milestone 1 — `CameraPage.qml` set `autoOrientation: true` on `VideoOutput`, a Qt 5 property that does not exist in Qt 6 (verified against Qt 6.11 docs; Qt 6 offers `orientation`/`mirrored` and handles camera frame rotation via frame metadata). Qt aborts the whole `main.qml` load on an invalid property, so this uncommitted redesign leftover masked everything. Removed the line as a minimal blocking fix.
  Evidence: CSV log `qrc:/CameraPage.qml:46:9: Cannot assign to non-existent property "autoOrientation"` followed by `QQmlApplicationEngine failed to load component`; clean startup after removal.
- Observation (M1, 2026-07-03): Binding `TabButton.width` to `tabBar.width` creates an `implicitWidth` binding loop once the buttons use the built-in `icon`/`text` content (TabBar derives its implicit size from its buttons). Fixed by binding to the enclosing container (`tabBarContainer.width / 4`) instead.
  Evidence: `QML TabBar: Binding loop detected for property "implicitWidth"` in the CSV log; zero occurrences after the change.
- Observation (M1, 2026-07-03): `SettingSwitcher.qml:15` emits a non-fatal "Duplicate signal name" warning at startup (a declared signal shadows a superclass/property-change signal). Pre-existing, not M1 scope; flagged for a separate fix.
- Observation (M5, 2026-07-03): Qt Quick Controls defaults to the native macOS style on desktop, which silently ignores `contentItem`/`background` customization ("The current style does not support customization of this control"). Since the whole UI is custom-themed from `Theme.qml`, the app now pins `QQuickStyle::setStyle("Basic")` in `main.cpp` (new `Qt6::QuickControls2` link). This also makes desktop rendering match what Android will do.
  Evidence: warning in the CSV log at first LivePage launch; zero occurrences after pinning the style.
- Observation (M5, 2026-07-03): The Metal RHI backend does not support line widths other than 1 px, so `QSGGeometry::setLineWidth(2)` logs a warning and renders 1 px anyway. The call was removed; if a fatter trace is wanted, the Milestone 6 polish pass should build triangle-strip ribbons instead of line strips.
  Evidence: "Line widths other than 1 are not supported by the graphics API" in the CSV log.
- Observation (M5, 2026-07-03): First live-fire worked end to end — during the smoke run the maintainer pressed Start: auto-connect engaged, the mercury simulator streamed at 100 ms intervals for ~34 s, Stop cleanly halted it. CSV log lines 22:29:18–22:29:52.
- (More to be filled during implementation.)

## Decision Log

- Decision: All protocol/driver/analysis code is C++ (no Rust/C bindings). Protocol drivers are structured "sans-IO": a codec class that only translates bytes to/from readings and never touches a socket, serial port, or BLE handle. Transports (simulated, BLE, later USB) are separate QObject classes that move bytes.
  Rationale: Maintainer choice (2026-07-03) to stay in one toolchain; the sans-IO boundary keeps the door open for other languages later and makes codecs unit-testable against captured byte traces.
  Date/Author: 2026-07-03 / Claude + maintainer
- Decision: Adopt a Nikon-inspired dark instrument design: matte near-black background, dark grey surfaces, one yellow accent (`#FFE100`) used sparingly (selection, active values, primary actions, the live trace), red reserved for record/destructive, hairline separators, uppercase micro-labels with letter-spacing, and monospaced tabular numerals for readouts. This supersedes the light/teal palette from the earlier iOS-redesign work; `Theme.qml` token names are kept so all existing pages retheme automatically.
  Rationale: Maintainer request (2026-07-03) for a "modern design like Nikon"; a dark UI is also the correct choice for an optical instrument used in dim labs and preserves night vision in the field.
  Date/Author: 2026-07-03 / Claude + maintainer
- Decision: Simulation is a first-class, separate shared library (`spectro-sim`, Milestone 3) rather than a single hard-coded fake device: it contains a physics-based `SpectrumSynthesizer`, a parameterized `InstrumentModel` (integration-time scaling, shot/read noise, dark current, hot pixels, 16-bit saturation, deterministic seeding), simulated devices built from scenes, a scriptable byte transport, fault injection, a byte-level impersonation of the future BLE bridge, and trace record/replay.
  Rationale: Maintainer request (2026-07-03) for a simulation library enabling all development and CI without hardware. Deterministic seeds make tests reproducible; the synthesizer's known ground truth turns analysis tests into scientific validation; the byte-level bridge loopback doubles as the executable specification of the firmware contract.
  Date/Author: 2026-07-03 / Claude + maintainer
- Decision: The device stack below the modality layer is sensor-neutral: the byte transport is `SensorTransport`, devices derive from a `SensorDevice` base (`SpectrometerDevice` and `GeigerDevice` are specializations), the codec sink delivers a `SensorReading` variant (`std::variant<Spectrum, GeigerReading>`), and the bridge wire contract carries a frame-type byte so one link can serve multiple instruments.
  Rationale: Maintainer decision (2026-07-03) to support additional hardware (Geiger counter now, possibly A-scan ultrasound later). Renaming and the variant cost nothing before implementation starts and make each new modality purely additive. Note: the existing `FrameProcessor` plugin system is an extension point for camera-frame processing, not for hardware — sensors plug in here, at the transport/codec/device layer.
  Date/Author: 2026-07-03 / Claude + maintainer
- Decision: Geiger counter support is in scope as Milestone 10: simulated end-to-end (seeded Poisson tube scenes, dose-rate card on the Live tab, strip chart, `measurements` table, CSV export, dose line in the video overlay); real-hardware acceptance gated. Scalar time-series data gets its own `measurements` table rather than being forced into the spectrum-shaped `spectra` table.
  Rationale: Maintainer request (2026-07-03). A second modality is the honest proof that the sensor-neutral stack is actually neutral, and radiation is the cheapest professional-feeling modality to simulate faithfully (radioactive decay is exactly a Poisson process).
  Date/Author: 2026-07-03 / Claude + maintainer
- Decision: Ultrasound is deferred, split by kind. A-scan ultrasound (echo amplitude vs. time-of-flight — thickness gauges, rangefinders) is recorded as a likely future extension: it is structurally a spectrum (two equal-length arrays with a different x-axis), and the hooks it needs are built by this plan (sensor-neutral stack, `SpectrumView` configurable x-axis, frame-type byte). Imaging/B-mode ultrasound is explicitly out of scope for this plan and its successors: megabyte-per-second data rates exceed BLE entirely and commercial imaging probes ship proprietary mobile SDKs — a separate product decision, not a sensor addition.
  Rationale: Maintainer decision (2026-07-03).
  Date/Author: 2026-07-03 / Claude + maintainer
- Decision: Render spectra with a custom `QQuickItem` (`SpectrumView`) using scene-graph geometry nodes, not Qt Charts / Qt Graphs.
  Rationale: Full control, guaranteed availability (no dependency on optional Qt modules being present in the local kit), and the only reliable way to redraw a 2048-point polyline at preview rates without per-frame QML data marshaling. The view is fed from C++ by connecting directly to `SpectrometerService`, honoring the thin-view rule.
  Date/Author: 2026-07-03 / Claude
- Decision: Analysis stages (smoothing, baseline, absorbance, peaks, integration) are built into `spectro-core` and toggled from the UI — they are not runtime plugins. The existing `FrameProcessor` plugin system stays untouched and is reused only for the video overlay.
  Rationale: The plugin machinery exists for camera-frame filters; spectral math is core product behavior, is needed on iOS where runtime plugin loading is unavailable anyway, and is far easier to test as plain functions.
  Date/Author: 2026-07-03 / Claude
- Decision: New database tables and DAOs live in `camera-core` next to the existing ones, because `DatabaseManager` lives there. The library is not renamed in this plan.
  Rationale: Avoids cross-library churn; renaming targets is cosmetic and can be its own later change.
  Date/Author: 2026-07-03 / Claude
- Decision: Tab set becomes six tabs: Live, Sessions, Camera, Photos, Videos, Settings — Live first and default. On window widths ≥ 900 px the Live tab shows a right-hand analysis side panel (multi-panel tablet layout); below that width the panel is a slide-in drawer. The Geiger readout is a card on the Live tab, not a seventh tab.
  Rationale: Spectroscopy is the product; the camera/gallery features remain useful for documentation. Six tabs are comfortable on tablets, the primary target.
  Date/Author: 2026-07-03 / Claude
- Decision: Milestones 1–8, all of Milestone 10's simulated scope, and Milestone 9's loopback scope are fully verifiable on the development Mac. The live-hardware acceptances of Milestones 9 and 10 are explicitly marked "gated". Milestone 11 (Android) must not be started unless a Qt for Android kit and a device are available. iOS USB-probe support (via a hardware bridge) is out of scope and will be a separate ExecPlan.
  Rationale: Every milestone must be independently verifiable by the implementing agent; hardware may not be on the desk.
  Date/Author: 2026-07-03 / Claude
- Decision (M2 refinement): The typed ready signals (`spectrumReady`, `readingReady`) are declared on the `SensorDevice` base itself, not on the `SpectrometerDevice`/`GeigerDevice` subclasses as originally sketched. `SpectrometerDevice` remains as a semantic subclass (Milestone 3's simulator derives from it); `GeigerDevice` is deferred to Milestone 10. `CodecDevice` also takes an explicit `QString name` first constructor argument (a codec+transport pair has no intrinsic display name).
  Rationale: Qt's moc cannot template signals, and the generic `CodecDevice` must emit whichever `SensorReading` alternative its codec delivers — with the signals on the base, every service connects to one uniform surface and a device simply never emits the signals that don't apply. The alternative (per-modality CodecDevice subclasses with duplicated wiring) buys type purity at the cost of real duplication.
  Date/Author: 2026-07-03 / Claude (implementing M2)
- Decision: Implementation is driven by four project subagents defined in `.claude/agents/` — `spectro-architect` (milestone planning and interface design), `spectro-builder` (implementation), `spectro-reviewer` (convention and correctness review of each milestone diff), `spectro-scientist` (test authoring and scientific validation against simulation ground truth). The agent definitions embed the repository conventions so each starts correctly without this conversation's context.
  Rationale: Maintainer request (2026-07-03). Role separation keeps plan maintenance, code quality, and numerical validation from being skipped under implementation momentum.
  Date/Author: 2026-07-03 / Claude + maintainer

## Outcomes & Retrospective

- (2026-07-03) Planning complete; implementation not started.

## Context and Orientation

The repository root contains `QML_ML_Camera/`, built with CMake into these targets: `logger-core` (a shared library that writes CSV logs via Qt logging categories; use `qDebug(logInfo()) << ...` etc. from `loggingcategories.h`, never bare `qDebug()`), `processor-api` (a header-only interface, `QML_ML_Camera/processor-api/FrameProcessor.h`, for runtime-loaded camera-frame filter plugins), `camera-core` (a shared library holding `DatabaseManager` — a class that opens the SQLite database — plus DAO classes that run raw SQL and `QAbstractListModel` subclasses that expose rows to QML), `plugins/grayscale` (a sample frame-processor plugin), the executable `QML_ML_Camera_App` in `QML_ML_Camera/app/`, and a CTest suite in `QML_ML_Camera/tests/`.

In `QML_ML_Camera/app/`, `main.cpp` constructs the models and services and registers them with the QML engine: context properties `albumModel`, `pictureModel`, `movieModel`, `loggerModel`, `pluginModel`, `logsPath`, and singletons `CameraService` and `AppSettings` under the QML import `solid.broccoli 1.0`. `CameraService` (`CameraService.h/.cpp`) is the pattern to imitate: a single C++ QObject owning the whole capture pipeline, exposing Q_PROPERTYs and Q_INVOKABLEs so QML never constructs device objects. It also owns a worker thread (`FrameProcessingWorker`) that runs `FrameProcessor` plugins on live video frames off the GUI thread. `AppSettings` persists user toggles with `QSettings`. `StorageLocations` (`camera-core`) returns per-platform data directories (pictures, recordings, logs).

The QML shell: `main.qml` is an `ApplicationWindow` with a bottom `TabBar` and a `StackLayout` holding one `StackView` per tab (currently Photos, Videos, Camera, Settings). `NavPage.qml` is the reusable page base (top navigation bar with title, back chevron, trailing action slot). `Theme.qml` is a `pragma Singleton` design-token file (colors, type scale, metrics) registered in `qmldir`; `Style.qml` is a legacy shim whose properties now point at `Theme` tokens, kept so old pages still build. New QML files must be added to the `qt_add_resources` FILES list in `QML_ML_Camera/app/CMakeLists.txt` or `qrc:/` loads fail at runtime.

Terms used below. A "spectrum" is a pair of equal-length arrays: wavelengths in nanometres and measured intensity counts, plus acquisition metadata. A "dark" spectrum is captured with the light path blocked (sensor noise floor); a "reference" is captured through the blank/light source. "Transmittance" T = (sample − dark) / (reference − dark), per wavelength; "absorbance" A = −log10(T). The "Beer-Lambert law" says absorbance is proportional to absorber concentration, which is why the dye simulation exposes a concentration parameter. "Integration time" is how long the sensor collects light per reading (the exposure). A "Geiger counter" detects ionizing-radiation events; its data is a scalar event rate — counts per second (CPS) and per minute (CPM) — converted to an approximate dose rate in microsieverts per hour (µSv/h) by multiplying CPM by a tube-specific conversion factor. A "Poisson process" is the statistics of independent random events at a constant mean rate (radioactive decay is one); its defining property, variance ≈ mean, is used by the tests. A "transport" is a class that moves raw bytes to/from a device; a "codec" is a class that translates those bytes into readings and commands into bytes; a "sans-IO codec" means the codec never performs I/O itself, so it can be tested by feeding it byte arrays. "Ground truth" means the known-correct answer available because the simulator itself injected it (e.g. it placed a peak at exactly 546.1 nm, or configured a mean rate of 50 CPS). "Scene graph" is Qt Quick's GPU renderer; a `QQuickItem` can supply it geometry (`QSGGeometryNode`) directly, which is how `SpectrumView` draws thousands of line segments cheaply.

Build and run (from the repository root; Qt 6.11 via Homebrew, `/opt/homebrew/opt/qt`):

    cmake -S QML_ML_Camera -B QML_ML_Camera/build
    cmake --build QML_ML_Camera/build -j
    ./QML_ML_Camera/build/app/QML_ML_Camera_App.app/Contents/MacOS/QML_ML_Camera_App

Tests: `ctest --test-dir QML_ML_Camera/build --output-on-failure`. Lint QML with `/opt/homebrew/opt/qt/bin/qmllint QML_ML_Camera/app/*.qml`. The existing suite must stay green throughout.

## Plan of Work

Work proceeds in eleven milestones. Each ends with the app building, tests passing, and a behavior a human can observe. Introduce everything additively; the camera/gallery features must keep working at every boundary.

### Milestone 1 — Instrument design language

Goal: the entire existing app renders in the Nikon-style dark instrument theme, from one token file, before any spectroscopy code exists.

Edit `QML_ML_Camera/app/Theme.qml` in place, keeping every existing property name (pages bind to them) and changing values to the dark set. Colors: `accent: "#FFE100"` (the single yellow; Nikon's brand yellow), `accentPressed: "#C7B000"`, `textOverAccent: "#000000"` (black text on yellow, as on Nikon bodies), `destructive: "#FF453A"`, `destructivePressed: "#D93A30"`, `groupedBackground: "#0E0E10"` (matte near-black screen background), `surface: "#1A1A1D"` (cards, nav/tab bars), `label: "#F2F2F2"`, `secondaryLabel: "#98989F"`, `tertiaryLabel: "#5A5A60"`, `separator: "#2C2C30"`, `fill: "#242428"`. Add new tokens: `surfaceElevated: "#242428"` (sheets, popovers), `live: "#30D158"` (acquiring indicator), `traceLive: "#FFE100"`, `traceReference: "#64D2FF"`, `traceDark: "#8E8E93"`, `traceOverlay1: "#FF9F0A"`, `traceOverlay2: "#BF5AF2"`, `traceOverlay3: "#5AC8FA"`, `plotGrid: "#232327"`, `plotAxis: "#3A3A3F"`; `readoutFontName: "Menlo"` (monospaced; on non-mac platforms fall back by leaving QML `font.family` unset when the family is unavailable — acceptable for now) and `microLabelSpacing: 1.2` (letter-spacing for uppercase micro-labels like "PEAK λ"). Keep the type scale and metrics as-is.

The design intent, stated so every later screen follows it: backgrounds are matte and near-black, never pure black; surfaces separate by one hairline or a slightly lighter grey, never drop shadows; yellow appears only where the user's eye must go (active tab, primary button, selected value, the live trace, focus rings); numbers that change live (wavelengths, counts, timers) render in the monospaced readout font so they don't jitter horizontally; section labels are uppercase, small (`caption` size), `secondaryLabel` color, with `microLabelSpacing`; controls are flat with `radiusControl` corners; red means recording, radiation above the alert threshold, or destruction — nothing else.

Also in this milestone: tint the tab-bar SVG icons correctly. In `main.qml`'s `TabButton` the `Image` is currently untinted; replace the manual `Image` with the `TabButton`'s own `icon` properties (`icon.source`, `icon.color: tabBar.currentIndex === index ? Theme.accent : Theme.secondaryLabel`), which the codebase already uses elsewhere for recoloring single-color SVGs.

Acceptance: clean build; launching shows the same four tabs, but the whole app is dark with yellow accents: dark tab bar with yellow active icon+label, dark nav bars, dark grouped Settings, dark dialogs. No page logic changed; `ctest` green; `qmllint` clean.

### Milestone 2 — `spectro-core` library: domain types, analysis math, device abstractions

Goal: a new shared library holding everything sensor-related that does not depend on Qt Quick or on any concrete device, proven by unit tests — no UI yet.

Create `QML_ML_Camera/spectro-core/` with its own `CMakeLists.txt` modeled on `camera-core`'s (shared library, links `Qt6::Core` and `logger-core`), and add `add_subdirectory(spectro-core)` to `QML_ML_Camera/CMakeLists.txt` after `logger-core`. Files:

`Spectrum.h/.cpp`: value types (see Interfaces and Dependencies for exact signatures) — `AcquisitionParams` (integration time ms, averaging count), `Spectrum` (wavelength and counts vectors, capture timestamp, params, a `kind` enum: Sample/Dark/Reference), registered with `Q_DECLARE_METATYPE` so it can cross threads in queued signal connections. Also `GeigerReading` (timestamp, counts-per-second — implemented by Milestone 10 but defined now) and `SensorReading`, a `std::variant<Spectrum, GeigerReading>` that codecs deliver, so the codec interface never changes when a modality is added.

`SpectroAnalysis.h/.cpp`: pure functions. `savitzkyGolay(counts, windowSize, polyOrder)` — smoothing; implement the standard least-squares convolution coefficients for polyOrder 2–3 and odd windows 5–25 (precompute coefficients per (window, order) pair by solving the small Vandermonde normal equations with plain Gaussian elimination, ~40 lines). `findPeaks(spectrum, minProminence, minDistanceNm)` — local maxima above a prominence threshold (prominence = peak height minus the higher of the two flanking minima), returning wavelength/value pairs sorted by prominence. `integrate(spectrum, fromNm, toNm)` — trapezoidal rule over the wavelength range. `transmittance(sample, dark, reference)` and `absorbance(sample, dark, reference)` — per-element with clamping (denominator floor 1e-9; T clamped to [1e-6, 10] before log) so bad references cannot produce NaN/Inf into the renderer. All take/return `QVector<double>` or `Spectrum` and assert equal lengths.

`SensorDevice.h`: the abstract QObject base every hardware device implements — connect/disconnect/start/stop/params, `connectedChanged`, `errorOccurred` — deliberately ignorant of spectra so later modalities (the Geiger counter of Milestone 10, possibly A-scan ultrasound) reuse it unchanged. `SpectrometerDevice.h`: the spectrometer specialization adding the `spectrumReady(Spectrum)` signal (exact shape in Interfaces and Dependencies). `SensorTransport.h` and `ProtocolCodec.h`: the sans-IO pair (byte-level abstract classes; exact shape below) — a transport moves bytes for any sensor type, which is the point of the split. `CodecDevice.h/.cpp`: the generic device that wires a transport to a codec (bytes from the transport are fed to the codec; the codec delivers `SensorReading`s; the device surfaces each on the matching typed signal and forwards commands as encoded bytes). This class is what BLE (Milestone 9) and any future USB transport plug into unchanged.

Tests: new `QML_ML_Camera/tests/tst_spectroanalysis.cpp` — Savitzky-Golay preserves a pure quadratic exactly; findPeaks locates three synthetic Gaussians within ±1 sample; integrate of a constant equals width×value; absorbance of sample==reference is ~0 everywhere; clamping handles a zero reference without NaN. Register in `tests/CMakeLists.txt` following the existing pattern.

Acceptance: `cmake --build` succeeds; `ctest` runs `tst_spectroanalysis` among the suite, all green. Nothing user-visible yet — the proof is the tests failing before the code exists and passing after.

### Milestone 3 — `spectro-sim` library: the hardware-free laboratory

Goal: a simulation library good enough that every later milestone — and CI — needs no hardware, and good enough that analysis results can be checked against known physics.

Create `QML_ML_Camera/spectro-sim/` (shared library; links `spectro-core` and `Qt6::Core`; added to the top-level CMake after `spectro-core`). It has four layers, each independently usable:

Layer 1, `SpectrumSynthesizer.h/.cpp` — ideal physics, no noise, pure functions producing `QVector<double>` flux on a given wavelength grid. Provide: `mercuryLamp(grid, fwhmNm)` — Gaussian emission lines at 404.7, 435.8, 546.1, 577.0 and 579.1 nm with realistic relative intensities (546.1 dominant); `neonLamp(grid, fwhmNm)` — a handful of lines in the 585–705 nm range; `blackbody(grid, temperatureK)` — Planck's law, normalized, the broadband source for absorbance work; `gaussianAbsorber(grid, centerNm, fwhmNm, peakAbsorbance)` — an absorbance curve A(λ); and `applyBeerLambert(sourceFlux, absorbance, concentration)` — transmitted = source × 10^(−A·c). Also `standardGrid()` (2048 points, 340–1020 nm) and `as7265xGrid()` (the fixed 18-channel table: 410, 435, 460, 485, 510, 535, 560, 585, 610, 645, 680, 705, 730, 760, 810, 860, 900, 940 nm).

Layer 2, `InstrumentModel.h/.cpp` — what a real sensor does to ideal flux. A small class configured by an `InstrumentProfile` struct (seed, full-well counts = 65535 for 16-bit saturation, dark current counts/ms, read-noise sigma, hot-pixel indices, relative quantum-efficiency curve as a smooth bell over the grid) with one method `Spectrum measure(const QVector<double>& flux, const QVector<double>& grid, const AcquisitionParams& p)`: scales flux by integration time and QE, adds dark current × integration time, applies shot noise (Gaussian with sigma = sqrt(signal), the standard approximation of Poisson at these counts), read noise, hot pixels, clips to full well, then averages N independent draws when `averaging > 1`. The random generator is `std::mt19937` seeded from the profile — identical seeds must produce identical spectra, so tests are exactly reproducible.

Layer 3, devices and scenes. `SimScene.h` — a tiny struct naming a source (Mercury/Neon/Blackbody), an optional absorber (center, FWHM, peak A, concentration), and an amplitude drift (sinusoidal, period ~10 s, a few percent) so live views look alive. `SimulatedSpectrometer.h/.cpp` — a `SpectrometerDevice` needing no transport: on `start()` a `QTimer` fires every max(integrationTimeMs, 33) ms, evaluates the scene through the synthesizer and instrument model, and emits `spectrumReady`. Two canonical scenes ship as named constructors: `SimulatedSpectrometer::mercuryLamp()` (emission; for peak work) and `SimulatedSpectrometer::dyeSample()` (blackbody source through a Gaussian absorber centered 664 nm, FWHM 45 nm, peak A 0.8 at concentration 1.0 — methylene-blue-like; for absorbance work; its `captureReference` story: setting concentration to 0 via `setSceneConcentration(double)` yields the blank).

Layer 4, byte-level test doubles. `ScriptedTransport.h/.cpp` — a `SensorTransport` for tests: `enqueueIncoming(bytes, delayMs)` scripts what the "device" sends (delivered chunked, with configurable chunk size, so reassembly is exercised); everything written by the code under test is captured in `writtenBytes()` for assertion. `FaultPolicy` (a struct on both scripted and simulated transports): `dropAfterFrames` (emit `closed()` mid-stream), `corruptByteProbability` (flip a random byte, seeded), `stallMs` (silence gaps). `TraceRecorder.h/.cpp` — wraps any transport and appends timestamped rx/tx lines to a file; `ReplayTransport.h/.cpp` — plays such a file back with original timing (or `instant()` for tests). Trace format, one event per line: `<ms-offset> <rx|tx> <hex-bytes>`. (The byte-level impersonation of the BLE bridge itself, `SimulatedBridgeTransport`, is added in Milestone 9 next to the codec it validates, but it lives in this library; Milestone 10 extends it with Geiger frames.)

Tests, `tst_spectrosim.cpp` (this is the scientist's foothold — analysis validated against injected ground truth): mercury scene through `SpectroAnalysis::findPeaks` recovers 546.1 ± 0.5 nm as the top peak and finds ≥ 4 of the 5 lines; the dye scene measured at concentration 1.0 (sample), 0.0 (reference), and lamp-off (dark), pushed through `SpectroAnalysis::absorbance`, recovers peak absorbance 0.8 ± 0.05 at 664 ± 2 nm, and halving concentration halves the recovered absorbance within tolerance (Beer-Lambert linearity); identical seeds → bit-identical spectra; doubling integration time roughly doubles signal but not read noise; saturation clips at full well; averaging 4 reduces residual noise by ~2× (measure standard deviation against the noiseless synthesizer output); `ScriptedTransport` delivers scripted bytes in configured chunk sizes and honors `corruptByteProbability` deterministically under a fixed seed; `ReplayTransport` round-trips what `TraceRecorder` wrote.

Acceptance: `ctest` green including `tst_spectrosim`; no UI change. The proof of the milestone is reading the test output: the analysis pipeline demonstrably recovers the physics the simulator injected.

### Milestone 4 — `SpectrometerService` and device management

Goal: the app knows about spectrometers; from Settings you can connect a simulated device and see live state change.

Create `QML_ML_Camera/app/SpectrometerService.h/.cpp`, deliberately mirroring `CameraService`: one QObject owning device discovery, the active `SpectrometerDevice`, acquisition state, and the latest spectra (live, dark, reference, held sample). Exact API in Interfaces and Dependencies. Discovery lists the built-ins first: entry 0 "Simulated UV-Vis (mercury lamp)", entry 1 "Simulated dye sample (Beer-Lambert)"; Milestone 9 appends BLE scan results. The service stores the latest live `Spectrum` under a mutex (the overlay processor in Milestone 8 reads it from a worker thread), recomputes derived readouts on each frame (peak wavelength/value via `SpectroAnalysis::findPeaks`, frames-per-second as a rolling average), and emits `spectrumUpdated()`. Wire logging: connect/disconnect/start/stop/errors through `logInfo()`/`logCritical()`. The app target now links `spectro-core` and `spectro-sim`.

In `main.cpp`, construct `SpectrometerService` after `AppSettings` and register it: `qmlRegisterSingletonInstance("solid.broccoli", 1, 0, "SpectrometerService", &spectrometerService);`.

UI: add a SPECTROMETER section at the top of `SettingsPage.qml`: a device picker row (current device name; tapping cycles or opens a picker), a Connect/Disconnect button whose label and yellow/grey styling follow `SpectrometerService.connected`, an integration-time row (a `Slider` 5–1000 ms; value shown in the readout font), and an averaging row (1/2/4/8 segmented buttons). All rows bind to service properties and call invokables — no logic in QML.

Acceptance: build and run; Settings shows the SPECTROMETER section with both simulated devices selectable; pressing Connect flips the row state to Connected (yellow), the CSV log (`~/Library/Application Support/SolidBroccoli/SolidBroccoli/logs/`) shows the connect entry; adjusting integration time while connected updates the property (verified visually in Milestone 5; here via log line). `ctest` green.

### Milestone 5 — Live tab: the spectral viewfinder

Goal: the tricorder moment — a live, smooth, yellow spectral trace with instrument readouts.

Create `QML_ML_Camera/app/SpectrumView.h/.cpp`: a `QQuickItem` subclass registered in `main.cpp` via `qmlRegisterType<SpectrumView>("solid.broccoli", 1, 0, "SpectrumView")`. Properties: `source` (the `SpectrometerService*`; on set, connect to `spectrumUpdated`), `mode` (enum Raw/Transmittance/Absorbance mirroring the service), x/y view range properties for zoom/pan (`minWavelength`, `maxWavelength`, `minY`, `maxY`, plus `autoScaleY: bool`), axis configuration (`xAxisTitle`, default "λ / nm", and `xTickStep`, default 50 — kept generic so the same item can later plot echo time in µs for A-scan ultrasound), trace visibility flags (`showReference`, `showDark`), and an `overlays` list set from C++ (Milestone 7 reloads saved spectra into it). Implement `updatePaintNode()` building: a grid (vertical lines every `xTickStep`, horizontal at 5 divisions, `plotGrid` color, `plotAxis` for the zero/edge axes), the live polyline (one `QSGGeometryNode`, line-strip, `traceLive` yellow, 2 px), optional reference/dark polylines, overlay polylines cycling `traceOverlay1..3`, and (from Milestone 6) peak markers and the integration-region rectangle. Axis tick labels are QML `Label`s positioned by the item via invokable mapping functions (`wavelengthToX`, etc.) — text stays in QML where fonts are easy, geometry stays in C++.

Interaction, implemented inside `SpectrumView` with `QWheelEvent`/`QMouseEvent`/`QTouchEvent` (pinch via two-finger native gestures on macOS trackpad and touch pinch on tablets): wheel/pinch zooms wavelength around the cursor, drag pans, double-tap/double-click resets to full range and re-enables `autoScaleY`. Zoom state lives in the item; QML only displays it.

Create `QML_ML_Camera/app/LivePage.qml` (a `NavPage`, title "Live", `largeTitle` off — this screen is chromeless like the Camera tab): the `SpectrumView` filling most of the page on `groupedBackground`; a top readout strip of four monospaced readouts on `surface` (PEAK λ in nm, PEAK counts, INTEGRATION ms, FPS — uppercase micro-labels above values, values in `readoutFontName`, peak values in yellow); a bottom control bar: a segmented RAW / TRANS / ABS mode switch, a large round Start/Stop button (yellow ring, black play/stop glyph; while acquiring, a small `live` green dot beside the FPS readout), and Dark / Ref / Hold buttons (Dark and Ref call `captureDark()`/`captureReference()` and show a small filled dot on the button once captured; Hold freezes the display via `service.setHold(true)` without stopping acquisition). TRANS and ABS segments are disabled (tertiary color) until both dark and reference exist — with a one-line hint label explaining why, because a professional instrument tells the operator what it needs.

Rework the tab bar in `main.qml` to the six-tab set: Live (new, index 0, default), Sessions (placeholder page this milestone), Camera, Photos, Videos, Settings. Reuse existing SVGs where sensible; add two new single-color SVGs `images/svg/spectrum.svg` (a peaks polyline) and `images/svg/sessions.svg` (a stacked-layers glyph) drawn in the same thin-line style as the existing icon set, listed in the app `CMakeLists.txt` resources.

Acceptance: run the app → it opens on Live; pressing Start when disconnected auto-connects to the currently selected simulated device (implement this convenience in the service, logged); with the mercury device a moving yellow trace with five visible lines renders over the grid, PEAK λ reads ≈546 nm and FPS ~30 at default settings; raising integration time in Settings visibly raises amplitude and lowers FPS; wheel-zoom into 500–600 nm works and double-click resets; with the dye device, Dark then Ref (service invokable sets the scene blank, captures, restores — one button for the operator) then ABS shows the 664 nm absorption band. `ctest` green; `qmllint` clean.

### Milestone 6 — Analysis panel

Goal: the numbers a spectroscopist actually wants, computed live, in a tablet-worthy side panel.

Extend `SpectrometerService` with analysis state: `smoothingWindow` (0 = off, else odd 5–25), `peakProminence` (double, sensible default derived from the current amplitude), integration region (`integrationFromNm`/`integrationToNm`, NaN = unset), and derived results recomputed per frame in C++: `peaks` (a small `QAbstractListModel`, `PeakListModel`, rows = wavelength/value/prominence strings pre-formatted for display) and `integralValue` (double). Absorbance/transmittance computation moves fully into the service (`displaySpectrum()` returns the mode-transformed, optionally smoothed spectrum; `SpectrumView` renders exactly what the service hands it, so view and analysis can never disagree).

Create `QML_ML_Camera/app/AnalysisPanel.qml`: a `surface` panel with uppercase section labels — PROCESSING (smoothing on/off + window stepper; the existing `SettingSwitcher` restyles automatically from Theme), PEAKS (a compact `ListView` over the peak model: λ in yellow readout font, counts, prominence; tapping a row calls `view.centerOn(wavelengthNm)` to zoom the plot around that peak), INTEGRATION (two readout fields showing the region, a "Set from view" button that copies the current visible range, and the live integral in the readout font), and CAPTURE (a yellow "Save capture" primary button — enabled once acquiring; wired in Milestone 7, this milestone it logs a stub line). In `LivePage.qml`: when `window.width >= 900` the panel docks right at 320 px wide (plot and panel side by side — the multi-panel tablet layout); otherwise a right-edge `Drawer` opened by an ellipsis button in the control bar.

`SpectrumView` additions: peak markers (small yellow triangles + λ labels above the trace at each model row) and the integration region as a translucent yellow rectangle between the region bounds.

Tests: `tst_spectrometerservice.cpp` driving the service with the simulated devices over `QSignalSpy` (set mode without dark/ref → stays Raw and signals an error string; with dark+ref → absorbance array matches `SpectroAnalysis::absorbance` output; integration region produces a finite, stable integral; with the dye scene the integral over 600–730 nm is within tolerance of the ground-truth band area — reusing the Milestone 3 validation approach at the service level).

Acceptance: run; on the wide desktop window the Live tab shows plot + analysis panel side by side; peaks list shows the mercury lines; tapping a peak zooms to it; enabling smoothing visibly calms the noise without moving peak positions; setting the integration region from a zoomed view shows a stable integral value. `ctest` green including the new test.

### Milestone 7 — Sessions and export

Goal: captures become durable, tagged, reloadable, exportable data — the "lab notebook" half of the product.

Schema, in `camera-core` following the existing DAO pattern exactly (`AlbumDAO` is the reference): new `SessionDAO` (table `sessions`: id, name, created_utc, notes) and `SpectrumDAO` (table `spectra`: id, session_id FK, kind TEXT sample|dark|reference, name, created_utc, integration_ms, averaging, wavelengths BLOB, counts BLOB, tags TEXT). Blobs are raw little-endian float64 arrays; write a small `SpectrumCodecSql` helper pair (serialize/deserialize `QVector<double>` ↔ `QByteArray`) with a round-trip unit test. `DatabaseManager` gains the two DAOs and creates the tables in its init path. New `SessionModel` and `SessionSpectrumModel` (`QAbstractListModel`s; the latter scoped to a session id like `PictureModel` is scoped to an album). Register both as context properties in `main.cpp` (`sessionModel`, `sessionSpectrumModel`).

Wire capture: `SpectrometerService::saveCapture(QString name, QString tags)` snapshots the current sample (plus the dark and reference if present and not yet stored this session) into the current session via the models; if no session is active, create one named from the date ("Session 2026-07-03 14:02"). The Analysis panel's Save button opens the app's standard input dialog for name + comma-separated tags.

`SessionsPage.qml` replaces the placeholder tab: an inset list of sessions (name, date, capture count) → pushes a session detail page listing captures (kind badge — DARK/REF badges in grey, samples in white — name, time, tags) with trailing actions: Overlay (adds the capture to `SpectrumView.overlays`, up to three, cycling the overlay trace colors; a second tap removes), Export, Rename, Delete (destructive red, confirm dialog). Export writes two files to a new `StorageLocations::exportsDir()` (create the helper beside `picturesDir()`): `<name>.csv` (header comment lines with metadata, then `wavelength_nm,counts` rows) and `<name>.json` (full metadata + arrays via `QJsonDocument`), then reveals the folder via `QDesktopServices::openUrl` on desktop. Export logic is a small `SpectrumExporter` class in `spectro-core` — testable, and tested (round-trip: export then parse the CSV back, compare arrays).

Tests: `tst_sessiondao.cpp` following `tst_databasemanager.cpp`'s pattern (create session, insert spectrum blob, read back identical arrays; delete cascades), plus the exporter round-trip test.

Acceptance: run; Save capture from Live with tags "demo,mercury"; Sessions tab shows the session with the capture; Overlay renders it in orange behind the live yellow trace; Export produces a CSV whose first data line matches the ~340 nm start; restart the app and the session is still there and reloadable. `ctest` green with the new tests.

### Milestone 8 — Video documentation with spectrum overlay

Goal: record a camera video with the live spectrum drawn on it — the documentation feature that makes field work reportable.

Create `QML_ML_Camera/app/SpectrumOverlayProcessor.h/.cpp` implementing the existing `FrameProcessor` interface (`QML_ML_Camera/processor-api/FrameProcessor.h`) as an app-internal processor (constructed in `main.cpp`, not a loaded plugin): its `process(QImage)` paints, with `QPainter`, a semi-transparent dark panel in the lower third with the current trace polyline (yellow), min/max wavelength labels, peak-λ readout, and a timestamp. It reads the latest spectrum through a `std::function<Spectrum()>` snapshot getter given at construction (bound to `SpectrometerService::latestSpectrumSnapshot`, which copies under a mutex — `process()` runs on the camera worker thread, so this getter is the only cross-thread touch point).

Wire an `AppSettings` toggle `spectrumOverlay` (default off) plus a row in the Camera tab's settings sheet and in Settings. In `main.cpp`, compose the processor list: the pipeline set on `CameraService` becomes plugin processors + (overlay processor if enabled), rebuilt on either `PluginManager::enabledProcessorsChanged` or the new toggle (a small lambda combining both sources replaces the current direct connection). `CameraService` is untouched.

Link recordings to sessions: on `CameraService::recordingSaved`, if a spectroscopy session is active, also insert a row into a new `session_videos` table (id, session_id, filepath, duration_ms — add to `SessionDAO`), and show linked videos at the bottom of the session detail page (tap → push the existing `MoviePage` player).

Acceptance: enable the overlay toggle, start spectrometer acquisition, open Camera, record ~5 s of video; play it back from the Videos tab (or the session detail) — the recorded file itself shows the moving trace burned in (verify by playback, since the overlay runs before encode); with the toggle off, recording is pristine and (with no plugins enabled) the zero-overhead direct path is active. Existing camera tests stay green.

### Milestone 9 — BLE transport and AS7265x codec (hardware acceptance gated)

Goal: the app can acquire from a real Bluetooth LE spectral sensor; everything except the final live-hardware check is verified against the simulated bridge.

Add `find_package` component `Bluetooth` and link `Qt6::Bluetooth` in `spectro-core`. Create `BleTransport.h/.cpp` (a `SensorTransport`): constructed with a `QBluetoothDeviceInfo`, it owns a `QLowEnergyController` (central role), discovers the agreed GATT service, subscribes to the data characteristic's notifications (each notification's payload is emitted as `bytesReceived`), and writes commands to the control characteristic. All state transitions and errors go through `logInfo`/`logCritical` and the `transportError` signal.

The GATT contract this project owns (the ESP32 bridge firmware — out of scope here — must implement it): service UUID `5B0C0001-8E2B-4D8B-9C60-0A5B3D1EAD01`; data characteristic `5B0C0002-…AD01` (notify); control characteristic `5B0C0003-…AD01` (write). Data frames may span multiple notifications and are delimited by a 4-byte little-endian length prefix, followed by a common header — magic `0x5BEC` (u16), version `0x01` (u8), frame type (u8: `0x01` spectrum, `0x02` geiger, used by Milestone 10) — then a type-specific payload. Spectrum payload (type `0x01`): channel count (u8), timestamp ms (u32), then channelCount float32 counts. Control commands: `0x01` start, `0x02` stop, `0x03` set-params followed by u16 integration ms + u8 averaging.

Create `As7265xCodec.h/.cpp` in `spectro-core` (a `ProtocolCodec`): buffers incoming bytes, reassembles length-prefixed frames, validates magic/version/frame type (`0x01`), and for channelCount 18 maps counts onto `SpectrumSynthesizer::as7265xGrid()`'s wavelength table, delivering an 18-point `Spectrum` through the `SensorReading` sink. (18 points render fine in `SpectrumView`; verify markers/integration behave at low resolution.) Encode start/stop/params per the contract.

Create `SimulatedBridgeTransport.h/.cpp` in `spectro-sim`: a `SensorTransport` that impersonates the bridge firmware byte-for-byte — it parses incoming control commands (start/stop/params), and while "started" emits length-prefixed data frames at the configured rate, generated from a `SimScene` through the 18-channel grid and `InstrumentModel`, chunked to a configurable MTU (default 20 bytes) to mimic BLE notifications. It honors the `FaultPolicy`. This class is the executable specification of the firmware contract: the firmware is correct when the app cannot distinguish it from this transport. Register a third built-in device in `SpectrometerService`: "Simulated AS7265x bridge (loopback)" = `CodecDevice(SimulatedBridgeTransport, As7265xCodec)` — the full byte path with no radio.

Discovery: `SpectrometerService` gains a `QBluetoothDeviceDiscoveryAgent`; `refreshDevices()` scans (LE method, 5 s) and appends found devices advertising the service UUID after the simulated entries. On macOS, Bluetooth permission comes from the `NSBluetoothAlwaysUsageDescription` Info.plist key — add it via the app target's `MACOSX_BUNDLE` properties in `QML_ML_Camera/app/CMakeLists.txt`.

Tests, `tst_as7265xcodec.cpp` (the sans-IO payoff): feed hand-built byte arrays — one whole frame, a frame split across three chunks, two frames in one chunk, corrupt magic (skipped with error, stream resynchronizes on next length prefix) — asserting emitted spectra and encoded command bytes exactly; drive `CodecDevice` end-to-end with `ScriptedTransport` (start() → codec-encoded start bytes observed at the transport; scripted frame bytes in → `spectrumReady` out); drive it with `SimulatedBridgeTransport` at MTU 20 with `corruptByteProbability` 0.001 under a fixed seed → a known count of valid spectra arrives and the stream always resynchronizes; record a session via `TraceRecorder`, replay via `ReplayTransport`, identical spectra out.

Acceptance (simulated, required): `ctest` green including the codec tests; in the running app, connecting "Simulated AS7265x bridge (loopback)" shows a live 18-point spectrum on the Live tab through the full byte path. Acceptance (hardware, gated — record in Progress as blocked if no device): with an ESP32+AS7265x flashed to the contract, the device appears after Refresh in Settings, connects, and the Live tab shows an 18-point spectrum responding to light on the sensor.

### Milestone 10 — Geiger counter: the second sensor modality (hardware acceptance gated)

Goal: prove the sensor-neutral stack is actually neutral by adding radiation detection end-to-end — live dose-rate readout, scrolling count chart, sessions, export, video overlay — entirely against a simulated tube; real hardware is a gated extra.

Domain (`spectro-core`; the value types already exist from Milestone 2): `GeigerDevice.h` — the `SensorDevice` specialization with `readingReady(const GeigerReading&)`. `GeigerCodec.h/.cpp` — a `ProtocolCodec` that consumes the bridge contract's type-`0x02` frames (payload after the common header: u32 timestamp ms + float32 counts-per-second) and delivers `GeigerReading`s through the `SensorReading` sink; start/stop reuse the same control commands. Derived quantities are computed app-side, not in the codec: counts-per-minute as a 60-second rolling window, and dose rate in µSv/h as CPM × a configurable tube conversion factor (default 0.0057 µSv/h per CPM, the common approximation for the ubiquitous SBM-20 tube; persisted in `AppSettings` together with an alert threshold, default 0.5 µSv/h).

Simulation (`spectro-sim`): `SimulatedGeiger` — a `GeigerDevice` needing no transport; radioactive decay is a Poisson process, so the simulator draws event counts per tick from a seeded `std::poisson_distribution` at a scene-configured mean rate and emits readings at 2 Hz. Two named scenes: `background()` (≈0.3 CPS, i.e. ~18 CPM, natural background) and `checkSource()` (≈50 CPS with an optional slow drift, imitating a test source moving relative to the tube). Also extend `SimulatedBridgeTransport` with a geiger mode emitting type-`0x02` frames, so the byte path (bridge → codec → service) is testable without hardware, exactly as Milestone 9 did for spectra.

Service and UI: `QML_ML_Camera/app/GeigerService.h/.cpp`, the third and smallest service following the `CameraService`/`SpectrometerService` pattern (API in Interfaces and Dependencies), registered as a QML singleton; its device list holds "Simulated Geiger (background)", "Simulated Geiger (check source)", and "Simulated bridge (loopback, geiger)". `QML_ML_Camera/app/StripChartView.h/.cpp` — a scene-graph `QQuickItem` sibling of `SpectrumView` drawing a scrolling time series (window-length property, hairline grid, yellow trace, a red horizontal threshold line at the alert level). On the Live tab, when a Geiger device is connected, a RADIATION readout card appears under the readout strip: dose rate in the monospaced readout font (yellow normally, `destructive` red above the alert threshold), CPM beneath it, and a small sparkline; tapping the card pushes a full strip-chart page with Start/Stop and Save. A GEIGER section joins Settings (device picker, connect, tube factor, alert threshold). Persistence: `MeasurementDAO` in `camera-core` (table `measurements`: id, session_id FK, created_utc, type TEXT, value REAL, unit TEXT, summary TEXT JSON for windowed stats); `saveMeasurement` stores the current windowed summary (avg/max CPM, dose) into the active session; the session detail page lists measurements with a dosimeter badge; CSV export gains a `timestamp,cps,cpm,usv_per_h` variant. Video overlay: `SpectrumOverlayProcessor` adds a dose-rate line to its readout block whenever the Geiger service is acquiring — radiation readings burned into documentation video.

Tests (`tst_geiger.cpp`): with a fixed seed, the recovered mean rate matches the configured scene rate within tolerance and the sample variance ≈ the mean (the defining Poisson property — this modality's ground-truth check); rolling-CPM correctness against a hand-computed sequence; dose conversion and threshold flagging; codec: type-`0x02` frames parse, an interleaved stream of type-`0x01` and type-`0x02` frames dispatches each reading to the correct typed signal, and resynchronization after corruption still holds; `MeasurementDAO` round-trip.

Acceptance (simulated, required): run the app; connect "Simulated Geiger (background)" in Settings → the Live tab shows the RADIATION card at ~18 CPM wobbling Poisson-style; switching to the check-source device jumps the rate and turns the dose readout red above the threshold; the strip-chart page scrolls; Save stores a measurement visible in the session; export produces the CSV. Acceptance (hardware, gated — record as blocked if absent): a real tube behind the bridge firmware streams type-`0x02` frames end-to-end. Optional desktop-only stretch, only if the hardware is on the desk: a GQ GMC-series USB Geiger counter via a `QSerialPort`-based `SerialTransport` (adds the Qt `SerialPort` component to the build; desktop platforms only — Android/iOS USB serial is out of scope per the Decision Log).

### Milestone 11 — Android tablet bring-up (gated)

Do not start this milestone unless a Qt for Android kit (Qt 6.11 Android, JDK, SDK/NDK) and a physical Android tablet are present; otherwise mark it blocked in Progress and stop the plan here as complete-for-desktop. Scope when unblocked: a CMake preset for the Android kit; `AndroidManifest.xml` with BLE permissions (`BLUETOOTH_SCAN`/`BLUETOOTH_CONNECT` with `neverForLocation`, camera, microphone); runtime permission requests via `QBluetoothPermission`/`QCameraPermission` in the services before first use; a layout pass verifying the ≥900 px side-panel mode on the tablet and touch pinch-zoom in `SpectrumView`; and re-running the Milestone 9 and 10 hardware acceptances on the tablet. iOS (and the USB-probe bridge device it requires) is intentionally a separate future ExecPlan.

### Future extensions (recorded, not scheduled)

A-scan ultrasound (echo amplitude vs. time-of-flight, as in thickness gauges and rangefinders) is structurally a spectrum — two equal-length arrays with a different x-axis — and the hooks it needs are built by this plan: the sensor-neutral device stack (Milestone 2), `SpectrumView`'s configurable x-axis (Milestone 5), and the frame-type byte in the bridge contract (Milestone 9; a future type `0x03` would carry echo waveforms). When it becomes real, it is an additive milestone shaped like Milestone 10. Imaging (B-mode) ultrasound is out of scope permanently for this plan: megabyte-per-second data rates exceed BLE entirely, and commercial imaging probes ship proprietary mobile SDKs — pursuing it would be a separate product decision and ExecPlan, not a sensor addition here.

## Concrete Steps

All commands run from the repository root. After every milestone:

    cmake -S QML_ML_Camera -B QML_ML_Camera/build
    cmake --build QML_ML_Camera/build -j
    ctest --test-dir QML_ML_Camera/build --output-on-failure
    /opt/homebrew/opt/qt/bin/qmllint QML_ML_Camera/app/*.qml
    ./QML_ML_Camera/build/app/QML_ML_Camera_App.app/Contents/MacOS/QML_ML_Camera_App

Expected test transcript shape at Milestone 3 (counts grow in later milestones):

    100% tests passed, 0 tests failed out of N
    ... tst_spectroanalysis ... Passed
    ... tst_spectrosim ........ Passed

New QML files go into the `qt_add_resources` FILES list in `QML_ML_Camera/app/CMakeLists.txt`; new C++ files into the respective target's source list; new singletons/types are registered in `main.cpp` (C++ types) — `Theme`/`Style` stay in `QML_ML_Camera/app/qmldir`. Commit at every milestone boundary with the milestone name in the message. Update this plan's `Progress`, `Surprises & Discoveries`, and `Decision Log` at every stopping point.

Role separation (optional but recommended): the repository defines four subagents in `.claude/agents/` — `spectro-architect` (refines the next milestone against the current tree before coding), `spectro-builder` (implements it), `spectro-scientist` (authors/extends the tests, especially the ground-truth validations of Milestones 3, 6, 9 and 10), and `spectro-reviewer` (reviews the milestone diff against this plan and the repo conventions before commit). Each agent definition embeds the conventions; this plan remains the single source of truth they defer to.

## Validation and Acceptance

Cumulative end-state acceptance, all on the development Mac with no hardware: from a clean build, the app opens on a dark, yellow-accented Live tab; Start auto-connects the mercury-lamp simulator and renders a moving 2048-point trace at ~30 FPS whose dominant peak reads ≈546 nm; the dye device with Dark → Ref → ABS shows a 664 nm absorption band whose listed integral is stable; the analysis panel (docked at desktop width) lists the mercury lines, zooms to a tapped peak, and integrates a chosen region; Save capture persists into a session that survives restart; Export produces valid CSV and JSON; a camera recording made with the overlay toggle on plays back with the spectrum burned in; the "Simulated AS7265x bridge (loopback)" device streams an 18-point spectrum through the full byte-level codec path; the simulated Geiger counter shows a RADIATION card whose rate matches its scene, turns red above the alert threshold, and saves measurements into the session; `ctest` passes everything including the spectro/sim/DAO/codec/geiger tests; `qmllint` is clean; the pre-existing camera, gallery, and logger features all still work. The live-hardware checks of Milestones 9 and 10 and all of Milestone 11 are additionally gated on equipment as stated there.

## Idempotence and Recovery

Every milestone is additive (new targets, new files, new tables) except the `Theme.qml` value edits (M1) and the tab-set change in `main.qml` (M5), both trivially revertable via git. Database changes only add tables — `DatabaseManager` init uses CREATE TABLE IF NOT EXISTS per the existing pattern, so old databases upgrade in place and re-running is safe; no existing table is altered. CMake reconfiguration is idempotent. If a milestone is interrupted, the Progress section must be split into done/remaining before stopping; every boundary leaves the app building and tests green, so recovery is "read Progress, rebuild, continue".

## Artifacts and Notes

Expected CSV export shape (Milestone 7):

    # solid-broccoli spectrum export
    # name: mercury demo, kind: sample, captured: 2026-07-03T14:02:11Z
    # integration_ms: 100, averaging: 4, tags: demo,mercury
    wavelength_nm,counts
    340.00,102.41
    340.33,101.87
    ...

Expected bridge data frames (Milestones 9–10): 4-byte LE length, then `EC 5B` (magic 0x5BEC), `01` (version), then the frame type. Spectrum frame: `01` (type), `12` (18 channels), u32 timestamp, 18 × float32 counts. Geiger frame: `02` (type), u32 timestamp, float32 counts-per-second.

Trace file format (Milestone 3), one event per line:

    0 tx 04000000EC5B01...      # first write, ms-offset 0
    112 rx 1A000000EC5B0101...  # notification received 112 ms later

## Interfaces and Dependencies

Qt modules: existing (Core, Gui, Qml, Quick, Sql, Svg, Multimedia) plus `Bluetooth` from Milestone 9 (`find_package(Qt6 REQUIRED COMPONENTS ... Bluetooth)`, linked by `spectro-core`); `SerialPort` only if Milestone 10's optional desktop stretch is taken. No third-party libraries. Library dependency direction: `spectro-sim` → `spectro-core` → (`logger-core`, Qt Core); the app links both; tests link what they exercise.

In `QML_ML_Camera/spectro-core/Spectrum.h`:

    struct AcquisitionParams {
        int integrationTimeMs = 100;
        int averaging = 1;
    };

    class Spectrum {
    public:
        enum class Kind { Sample, Dark, Reference };
        QVector<double> wavelengthsNm;   // ascending, same length as counts
        QVector<double> counts;
        qint64 timestampMs = 0;          // ms since epoch
        AcquisitionParams params;
        Kind kind = Kind::Sample;
        bool isValid() const;            // equal non-zero lengths, ascending λ
    };
    Q_DECLARE_METATYPE(Spectrum)

    struct GeigerReading {
        qint64 timestampMs = 0;
        double countsPerSecond = 0.0;
    };
    Q_DECLARE_METATYPE(GeigerReading)

    using SensorReading = std::variant<Spectrum, GeigerReading>;

In `QML_ML_Camera/spectro-core/SpectroAnalysis.h` (namespace `SpectroAnalysis`): `QVector<double> savitzkyGolay(const QVector<double>& y, int window, int polyOrder);` `struct Peak { int index; double wavelengthNm; double value; double prominence; };` `QVector<Peak> findPeaks(const Spectrum& s, double minProminence, double minDistanceNm);` `double integrate(const Spectrum& s, double fromNm, double toNm);` `QVector<double> transmittance(const Spectrum& sample, const Spectrum& dark, const Spectrum& reference);` `QVector<double> absorbance(...same...);`

In `QML_ML_Camera/spectro-core/SensorTransport.h`:

    class SensorTransport : public QObject {
        Q_OBJECT
    public:
        virtual bool open(QString* errorMessage) = 0;
        virtual void close() = 0;
        virtual void writeBytes(const QByteArray& bytes) = 0;
    signals:
        void bytesReceived(const QByteArray& bytes);
        void transportError(const QString& message);
        void closed();
    };

In `QML_ML_Camera/spectro-core/ProtocolCodec.h` (sans-IO; no QObject needed):

    class ProtocolCodec {
    public:
        virtual ~ProtocolCodec() = default;
        virtual void feed(const QByteArray& bytes) = 0;      // may emit 0..n readings via sink
        virtual void setSink(std::function<void(SensorReading)> sink) = 0;
        virtual QByteArray encodeStart() = 0;
        virtual QByteArray encodeStop() = 0;
        virtual QByteArray encodeParams(const AcquisitionParams&) = 0;
    };

In `QML_ML_Camera/spectro-core/SensorDevice.h` (as built in M2): the abstract QObject base with `virtual bool connectDevice(QString* err)`, `disconnectDevice()`, `start()`, `stop()`, `setParams(const AcquisitionParams&)`, `QString name() const`, a non-virtual `bool isConnected()` with a protected `setConnected(bool)` helper, and signals `errorOccurred(const QString&)`, `connectedChanged(bool)`, `spectrumReady(const Spectrum&)`, `readingReady(const GeigerReading&)` — all typed ready signals live on the base (see Decision Log, M2 refinement); a device emits only those that apply. `SpectrometerDevice` is a semantic subclass (no added members); `GeigerDevice` (Milestone 10) likewise. `CodecDevice(QString name, std::unique_ptr<SensorTransport>, std::unique_ptr<ProtocolCodec>)` implements the base generically and emits the typed signal matching whichever `SensorReading` alternative the codec delivers.

In `QML_ML_Camera/spectro-sim/` (all classes exported like `camera-core`'s):

    namespace SpectrumSynthesizer {
        QVector<double> standardGrid();                       // 2048 pts, 340–1020 nm
        QVector<double> as7265xGrid();                        // 18 fixed channels
        QVector<double> mercuryLamp(const QVector<double>& grid, double fwhmNm);
        QVector<double> neonLamp(const QVector<double>& grid, double fwhmNm);
        QVector<double> blackbody(const QVector<double>& grid, double temperatureK);
        QVector<double> gaussianAbsorber(const QVector<double>& grid,
                                         double centerNm, double fwhmNm, double peakA);
        QVector<double> applyBeerLambert(const QVector<double>& sourceFlux,
                                         const QVector<double>& absorbance,
                                         double concentration);
    }

    struct InstrumentProfile {
        quint32 seed = 1;
        double fullWellCounts = 65535.0;      // 16-bit saturation
        double darkCurrentPerMs = 0.5;        // counts per ms integration
        double readNoiseSigma = 8.0;          // counts
        QVector<int> hotPixels;               // indices forced to full well
        double countsPerMsFullFlux = 400.0;   // scale: counts/ms at flux 1.0 (added in M3)
        double qeFloor = 0.3;                 // quantum-efficiency bell (added in M3)
        double qeCenterNm = 680.0;
        double qeSigmaNm = 300.0;
    };
    class InstrumentModel {
        // Spectrum measure(flux, grid, params) — deterministic per seed.
        // QVector<double> idealSignal(flux, grid, params) — noise-free
        // expectation, public so tests compute exact residuals (added in M3).
    };

    struct SimScene { /* source enum, optional absorber(center,fwhm,peakA,concentration), drift */ };
    class SimulatedSpectrometer : public SpectrometerDevice {
        static SimulatedSpectrometer* mercuryLamp(QObject* parent = nullptr);
        static SimulatedSpectrometer* dyeSample(QObject* parent = nullptr);
        void setSceneConcentration(double c);   // 0.0 = blank (for reference capture)
    };
    class SimulatedGeiger : public GeigerDevice {              // Milestone 10; seeded Poisson process
        static SimulatedGeiger* background(QObject* parent = nullptr);   // ~0.3 CPS
        static SimulatedGeiger* checkSource(QObject* parent = nullptr);  // ~50 CPS, optional drift
    };

    struct FaultPolicy { int dropAfterFrames = -1; double corruptByteProbability = 0.0;
                         int stallMs = 0; quint32 seed = 1; };
    class ScriptedTransport : public SensorTransport {   // enqueueIncoming(bytes, delayMs),
    };                                                   // writtenBytes(), chunkSize, FaultPolicy
    class SimulatedBridgeTransport : public SensorTransport { // byte-level bridge impersonation,
    };                                     // SimScene + InstrumentModel + MTU + FaultPolicy;
                                           // spectrum mode (type 0x01) or geiger mode (type 0x02)
    class TraceRecorder;   // wraps a transport, writes "<ms> <rx|tx> <hex>" lines
    class ReplayTransport : public SensorTransport;  // plays a trace file, or instant() for tests

In `QML_ML_Camera/app/SpectrometerService.h` (registered as QML singleton `SpectrometerService` in `solid.broccoli 1.0`): properties `QStringList availableDevices`, `int currentDeviceIndex`, `bool connected`, `bool acquiring`, `bool hold`, `int integrationTimeMs`, `int averaging`, `int mode` (0 raw, 1 transmittance, 2 absorbance), `bool hasDark`, `bool hasReference`, `double peakWavelengthNm`, `double peakValue`, `double framesPerSecond`, `double integralValue`, `QObject* peakModel`; invokables `refreshDevices()`, `connectDevice()`, `disconnectDevice()`, `startAcquisition()`, `stopAcquisition()`, `captureDark()`, `captureReference()`, `setIntegrationRegion(double fromNm, double toNm)`, `saveCapture(QString name, QString tags)`; signals `spectrumUpdated()`, `errorOccurred(QString)`; plus thread-safe `Spectrum latestSpectrumSnapshot() const` (mutex-guarded copy) for the overlay processor, and `const Spectrum& displaySpectrum() const` + overlay accessors for `SpectrumView` (GUI thread only).

In `QML_ML_Camera/app/GeigerService.h` (Milestone 10; QML singleton `GeigerService` in `solid.broccoli 1.0`): properties `QStringList availableDevices`, `int currentDeviceIndex`, `bool connected`, `bool acquiring`, `double countsPerSecond`, `double countsPerMinute` (60 s rolling window), `double doseMicroSvPerHour` (CPM × tube factor), `double tubeFactor`, `double alertThresholdUSvPerHour`, `bool aboveThreshold`; invokables `connectDevice()`, `disconnectDevice()`, `startAcquisition()`, `stopAcquisition()`, `saveMeasurement(QString name, QString tags)`; signal `readingUpdated()`. Tube factor and alert threshold persist via `AppSettings`; a thread-safe `GeigerReading latestReadingSnapshot() const` serves the video overlay. `QML_ML_Camera/app/StripChartView.h`: a `QQuickItem` sibling of `SpectrumView` drawing a scrolling time series (`windowSeconds`, `thresholdValue` properties; fed from `GeigerService`).

`QML_ML_Camera/app/SpectrumView.h`: `class SpectrumView : public QQuickItem` with the properties listed in Milestone 5 (including the generic `xAxisTitle`/`xTickStep` axis configuration), `Q_INVOKABLE void centerOn(double wavelengthNm)`, `Q_INVOKABLE double wavelengthToX(double nm) const`, and `updatePaintNode()` doing all drawing.

DAOs (in `camera-core`, mirroring `AlbumDAO`'s constructor-takes-`QSqlDatabase&`, `init()`, CRUD shape): `SessionDAO` (`addSession`, `sessions()`, `updateName`, `updateNotes`, `removeSession` — cascades spectra, measurements, and video links; `addVideo(sessionId, filepath, durationMs)`, `videos(sessionId)`), `SpectrumDAO` (`addSpectrum(sessionId, const Spectrum&, name, tags)`, `spectra(sessionId)`, `removeSpectrum`, `rename`), and `MeasurementDAO` (Milestone 10: `addMeasurement(sessionId, type, value, unit, summaryJson)`, `measurements(sessionId)`, `removeMeasurement`). Models `SessionModel`/`SessionSpectrumModel` follow `AlbumModel`/`PictureModel` exactly, exposed as context properties `sessionModel`/`sessionSpectrumModel`.

The video overlay implements the existing interface in `QML_ML_Camera/processor-api/FrameProcessor.h` verbatim (`name()`, `description()`, `initialize(QString*)`, `QImage process(const QImage&)`); it must not block — if the spectrum snapshot is empty it returns the frame unchanged. (This plugin interface is for camera-frame processing only; hardware sensors integrate via `SensorTransport`/`ProtocolCodec`/`SensorDevice`, never as frame plugins.)

Design tokens are the single source of the Nikon-style look and are defined exhaustively in Milestone 1; no page may introduce a hex color not present in `Theme.qml`.

---

Revision note (2026-07-03): Revision 1. The single `SimulatedSpectrometer` originally planned inside `spectro-core` (old Milestone 2) grew into a dedicated `spectro-sim` library with its own milestone (now Milestone 3): physics-based synthesis with known ground truth, a seeded instrument-noise model, scene-based simulated devices (mercury lamp + Beer-Lambert dye sample), scripted/replay transports with fault injection, and a byte-level `SimulatedBridgeTransport` that doubles as the executable firmware contract (Milestone 9). Subsequent milestones renumbered 4–10; acceptance criteria updated to use the richer simulated devices (real absorbance demo instead of a flat trace). Reason: maintainer request for a proper simulation library so all development, testing, and CI run without hardware, and so analysis results are validated against injected physics rather than eyeballed. The same revision records the creation of the four implementation subagents under `.claude/agents/`.

Revision note (2026-07-03): Revision 2. The device stack below the modality layer was made sensor-neutral before any of it is implemented: `SpectrometerTransport` renamed to `SensorTransport`; a `SensorDevice` base introduced beneath `SpectrometerDevice`; the codec sink now delivers a `SensorReading` variant (`std::variant<Spectrum, GeigerReading>`); and the bridge wire contract gained a frame-type byte (`0x01` spectrum, `0x02` geiger) in its common header. Geiger counter support was added as the new Milestone 10 (simulated Poisson tube scenes, dose-rate card, `StripChartView`, `measurements` table, CSV export, dose line in the video overlay; real-tube acceptance gated), and Android bring-up renumbered to Milestone 11. `SpectrumView` gained generic x-axis configuration in Milestone 5. A "Future extensions" section records A-scan ultrasound as a likely later addition (its hooks are now in place) and declares imaging/B-mode ultrasound permanently out of scope for this plan. Reason: maintainer decision to include a Geiger counter and keep the door open for ultrasound, after establishing that the existing `FrameProcessor` plugin system is a camera-frame extension point, not a hardware one — hardware plugs in at the transport/codec/device layer, which this revision makes officially multi-sensor.
