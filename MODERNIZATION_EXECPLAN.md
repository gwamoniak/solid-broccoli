# Modernize the camera desktop application: architecture, capture pipeline, plugin system, tests, and icons

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds. This document follows the repository guidance in `PLANS.md` (repository root) and must be maintained in accordance with it.

## Purpose / Big Picture

Today the application (built from `QML_ML_Camera/` with CMake, producing `QML_ML_Camera_App`) shows a live camera preview and can save still images, but the architecture is a partial Qt5-to-Qt6 port with duplicated responsibilities, a half-implemented video recorder, a global database singleton that blocks testing, raster PNG icons, and no tests at all. After this plan is complete, a contributor can:

- Pick a camera device and resolution/format from a settings panel in the UI and see the preview update live.
- Record video to disk with start/stop controls and see the recording appear in the movie album.
- Capture stills that are saved asynchronously, registered in the SQLite database with metadata, and immediately visible in the gallery with generated thumbnails.
- Drop a shared library implementing the frame-processor plugin interface into a `plugins/` folder and see it listed in the UI; enabling it transforms the live preview frames (the proof plugin is a grayscale filter; the interface is designed so OpenCV and deep-learning plugins can be added later without touching the application).
- Run `ctest` from the build directory and see a unit-test suite pass.
- See a consistent, modern SVG icon set across the UI.

## Progress

- [x] (2026-06-10) Milestone 1: Core architecture restructure — generated qmake files removed from git, `DatabaseManager` de-singletoned (public path constructor, `isOpen()`, `database()`, unique connection names, `defaultDatabasePath()`), `DatabaseManager&` injected into `AlbumModel`/`PictureModel`/`LoggerModel`, `StorageLocations` added to camera-core, `logger-core` takes an injected log directory, Movie* dead code moved to `QML_ML_Camera/attic/`. Verified: clean CMake build; app launched from `/tmp` creates DB/logs under `~/Library/Application Support/SolidBroccoli/SolidBroccoli/`.
- [x] (2026-06-10) Milestone 2: Camera service and device/format setup UI — `CameraService` (app layer) owns `QMediaCaptureSession`/`QCamera`/`QImageCapture`/`QMediaRecorder`, exposes `availableCameras`/`currentCameraIndex`/`availableFormats`/`currentFormatIndex`/`active` and `attachVideoOutput`/`setActive`/`captureImage`; registered via `qmlRegisterSingletonInstance("solid.broccoli", 1, 0, "CameraService", ...)`. Hot-plug fallback on `QMediaDevices::videoInputsChanged`. `CameraPage.qml` rewritten as thin view (VideoOutput + settings `Drawer` with two ComboBoxes + capture error label + shutter flash); `CameraProcessor.*` deleted. Verified: build + smoke run; log shows camera enumeration and default device selection. Device-switch and format-switch interaction still to be exercised by a human with camera permission granted.
- [x] (2026-06-10) Milestone 3: Video recording — `CameraService` gained `startRecording()`/`stopRecording()` (MPEG4/H264 with `QMediaFormat::isSupported` fallback, output `SolidBroccoli_VID_<timestamp>.mp4` under `StorageLocations::recordingsDir()`), `recording` and `recordingDuration` ("mm:ss") properties, and a `recordingSaved(filePath, durationMs)` signal. New `Movie`/`MovieDAO`/`MovieModel` in camera-core (`movies` table: id, album_id, url, name, duration, created_at; `CREATE TABLE IF NOT EXISTS`); `main.cpp` connects `recordingSaved` → `MovieModel::addRecording` and registers `movieModel`. CameraPage got a pulsing record toggle + live duration; `MovieAlbumPage.qml` rewritten as a recordings list, `MoviePage.qml` rewritten as a Qt6 `MediaPlayer`+`AudioOutput` player with seek/volume/rename/delete; MainPage recordings button re-enabled. Verified: build clean, app starts, `movies` table created, qmllint shows only pre-existing style warnings. End-to-end record→playback needs an interactive session with camera permission.
- [x] (2026-06-11) Milestone 4: Image capture and saving pipeline — new `CaptureCoordinator` (app layer) listens to `CameraService::imageSaved` and registers the saved file into `PictureModel`'s currently-loaded album (guarded so captures with no active album are logged, not orphaned); shutter flash and capture-error label were already present in `CameraPage.qml`. New `ThumbnailCache` (camera-core) generates a 400px-max-edge JPEG under `StorageLocations::thumbnailsDir()` keyed by an MD5 of the source path, regenerating when the source is newer. `PictureProvider` rewritten as a `QQuickAsyncImageProvider`: the row→path lookup runs on the GUI thread in `requestImageResponse`, decode/scale runs on a `QThreadPool` via a `PictureResponse` (QQuickImageResponse + QRunnable), so opening large albums no longer stalls the UI. Verified: standalone harness confirmed a 2000x1500 source yields a 400x300 cached JPEG with a stable cache path on repeat; clean build; app starts.
- [x] (2026-06-11) Milestone 5: Frame-processor plugin system — new header-only `processor-api` INTERFACE target with `FrameProcessor.h` (interface + `FrameProcessor_iid` "broccoli.FrameProcessor/1.0", documents the QImage↔cv::Mat conversion for future OpenCV plugins). New `grayscale-processor` SHARED Qt plugin in `plugins/grayscale/` (emitted to `<build>/app/plugins/`). New `PluginManager` (app layer, `QAbstractListModel`) scans exe-dir/bundle/app-data plugin folders with `QPluginLoader`, validates the IID from metadata before instantiating, persists enabled state in `QSettings` keyed by plugin name, and emits `enabledProcessorsChanged`. `CameraService` gained a frame tap: when ≥1 processor is enabled the session renders into an intermediate `QVideoSink`, each frame is mapped to `QImage` and handed (latest-only, drop-if-busy via an atomic flag) to a `FrameProcessingWorker` on its own `QThread`; the processed image is wrapped in a `QVideoFrame` and pushed to the QML `VideoOutput`'s sink; with no processors the session writes directly to the output (zero overhead). `CameraPage.qml` settings drawer gained a "Processors" `ListView` of name/description/`Switch` rows with help text noting captures stay original. `main.cpp` wires `PluginManager` ↔ `CameraService` and registers `pluginModel`. Verified: app discovers/loads/registers the grayscale plugin (log: "registered processor: Grayscale"); harness confirmed `process()` turns saturated red into equal-channel gray in RGBA8888. Live grayscale preview needs an interactive session with camera permission granted.
- [x] (2026-06-12) Milestone 6: Unit testing module — root list file gained `include(CTest)` and a `BUILD_TESTING`-guarded `add_subdirectory(tests)`. New `QML_ML_Camera/tests/` with six guiless Qt Test executables, each registered via `add_test`: `tst_storagelocations` (paths absolute/under-root/created via `setRootForTesting` + `QTemporaryDir`), `tst_databasemanager` (`:memory:` opens and creates albums/pictures/movies, two instances coexist with distinct connection names, bad path reports closed), `tst_albummodel` (insert/rename/remove with `QSignalSpy` on rowsInserted/dataChanged/rowsRemoved + persistence reload + out-of-range rejection), `tst_picturemodel` (insert/data roles/remove + pictures cascade-delete with their album via the AlbumModel→PictureModel wiring), `tst_pluginmanager` (loads the real grayscale plugin, skips a junk `.dylib` without crashing, enable toggle persists in isolated `QSettings` and signals), and `tst_frameprocessor_grayscale` (colored frame → equal-channel `Format_RGBA8888`, size preserved). Added a `loadPlugins(extraDirs)` testability hook on `PluginManager`. Verified: `ctest --output-on-failure` → 6/6 passed in ~2s.
- [x] (2026-06-12) Milestone 7: Modern SVG icon set — added 14 monochrome SVG glyphs under `QML_ML_Camera/app/images/svg/` (add-album, add-photo, back, camera, delete, gallery, home, log, movie, quit, record, rename, save, settings) covering every prior PNG use. `Style.qml` gained `iconColor`, `dangerColor`, and `iconSize`; every `icon.source` in the QML pages now points at an SVG with `icon.color` set (delete/record use `dangerColor`) and a uniform `icon.width/height: Style.iconSize`. `app/CMakeLists.txt` resources list swapped PNGs for SVGs; the 18 PNG files were `git rm`'d (kept in history). README rewritten with build/test instructions and an icon license note. Verified: clean build; `grep images/png app/*.qml` returns nothing; live screenshot of MainPage shows every icon rendering crisply and tinted by theme color.

## Surprises & Discoveries

(To be filled during implementation.)

- Observation: Camera ownership is currently duplicated — `QML_ML_Camera/app/CameraPage.qml` builds its own `CaptureSession`/`Camera`/`ImageCapture` in QML while `QML_ML_Camera/app/CameraProcessor.cpp` builds a second, unused set in C++ inside `setCamera()`. Only the QML session feeds the preview; the C++ one is dead weight.
  Evidence: `CameraPage.qml` lines 16–54 instantiate both `CameraProcessor` and a QML `CaptureSession`; `CameraProcessor::setCamera()` ignores its argument (`Q_UNUSED(vCamera)`).
- Observation: `CameraProcessor::recordMovie()` is a stub — it composes a filename and does nothing else; `stopRecording()` only flips a bool.
  Evidence: `QML_ML_Camera/app/CameraProcessor.cpp` lines 88–108, including the comment "the rest is not developed yet".
- Observation: `DatabaseManager` is a process-wide singleton with a hard-coded database filename, constructed implicitly on first use by the models. This makes the DAOs and models untestable in isolation and means tests would write to the real `solidBroccoli_Gallery.db`.
  Evidence: `QML_ML_Camera/camera-core/DatabaseManager.h` (`DATABASE_FILENAME = "solidBroccoli_Gallery.db"`, `static DatabaseManager& instance()`), and `AlbumModel.h` storing `DatabaseManager& m_sqlDB`.
- Observation: `MovieProcessor`/`MovieAlbum` C++ sources exist in `QML_ML_Camera/app/` and `QML_ML_Camera/camera-core/` but are not compiled — they are absent from both `CMakeLists.txt` files, and `MovieProcessor.h` references a `MovieModel` class that does not exist in the tree.
  Evidence: grep for `MovieProcessor` in `app/CMakeLists.txt` returns nothing; no `MovieModel.h` exists under `camera-core/`.
- Observation: All output paths (pictures, recordings, logs, database) are relative to the process current working directory, so launching the app from a different directory silently creates a fresh gallery.
  Evidence: `CameraProcessor::nextPicturePath()` uses `QDir::currentPath()`; `DATABASE_FILENAME` is a bare filename; `logger.h` uses `sLogFolderName = "logs"`.
- Observation (M1): `LoggerModel` silently depended on Qt's *default* SQL connection — its bare `QSqlQuery query;` statements and `setQuery(QString)` only worked because the singleton used the unnamed connection. Moving to named connections (needed for test isolation) required passing `m_sqlDB.database()` into every query and the two-argument `setQuery` overload.
  Evidence: pre-change `LoggerModel.cpp` lines 21, 55, 61.
- Observation (M2): Launching the unsigned binary from a shell yields "Access to camera not granted" (macOS TCC); the camera still enumerates and the default device is selected, so the service logic is verifiable headlessly, but live preview needs a user-granted permission prompt (the `NSCameraUsageDescription` is already in `Info.plist.in`).
  Evidence: smoke-run log 2026-06-10 21:20: "Access to camera not granted" followed by "Camera selected: MacBook Pro Camera".
- Observation (M1): Qt nests org name and app name, so the data root is `~/Library/Application Support/SolidBroccoli/SolidBroccoli/`. Harmless; left as-is.
  Evidence: smoke-run `find` output.
- Observation (M5): Inside a macOS `.app` bundle the executable lives at `Contents/MacOS`, so `QCoreApplication::applicationDirPath() + "/plugins"` does not resolve to the build tree's `app/plugins/` where `qt_add_plugin` emits the module. `PluginManager` initially loaded 0 processors despite the dylib existing.
  Evidence: first smoke run logged "PluginManager: loaded 0 processor(s)." after the plugin built to `build/app/plugins/libgrayscale-processor.dylib`. Fixed by scanning several candidate dirs (`appDir/plugins`, `appDir/../PlugIns`, `appDir/../../../plugins`, app-data `plugins`); the third reaches the build tree beside the bundle and the run then logged "registered processor: Grayscale".
- Observation (M4/M5): The grayscale `process()` and `ThumbnailCache` were validated with a throwaway harness compiled against `camera-core` plus the plugin's own MOC output (the `Q_OBJECT`/`Q_PLUGIN_METADATA` vtable is undefined without `moc_GrayscaleProcessor.cpp`). Loading the plugin via `QPluginLoader` from the harness failed with "shared library was not found" (an rpath quirk of the ad-hoc harness, not the plugin) — but the real application loads it cleanly, so plugin loading is proven by the app and the harness only exercised the pure logic.
  Evidence: harness output "Grayscale output pixel: QColor(ARGB 1, 0.372549, 0.372549, 0.372549) isGray: true" and "Thumbnail ... size: QSize(400, 300)".
- Observation (M6): The "pictures cascade-delete with their album" behavior is not a database foreign key — SQLite FKs are off by default and the schema declares none. It is wired at the model layer: `AlbumModel::removeRows` deletes only the album row, and a `PictureModel` connected to `AlbumModel::rowsRemoved` deletes pictures for *its currently loaded album* (`m_nAlbumID`). So the cascade only fires for the album the picture model is showing. `tst_picturemodel` reproduces this exactly: it `setAlbumId(id)`, adds pictures, removes the album row, then asserts both the model and `PictureDAO::picturesForAlbum(id)` are empty.
  Evidence: `tst_picturemodel.cpp::picturesCascadeDeleteWithAlbum` passes; `AlbumDAO::removeAlbum` issues only `DELETE FROM albums WHERE id = ...`.
- Observation (M6): The grayscale processor and `PluginManager` could be unit-tested in-process after all (the M4/M5 throwaway harness had hit an rpath snag). Compiling `GrayscaleProcessor.cpp` straight into `tst_frameprocessor_grayscale` (AUTOMOC supplies the plugin vtable) and pointing `PluginManager::loadPlugins({extraDir})` at `$<TARGET_FILE_DIR:grayscale-processor>` both work, and `QPluginLoader` loads the real built `.dylib` cleanly from the test.
  Evidence: `ctest` shows `tst_pluginmanager` and `tst_frameprocessor_grayscale` passing; the former loads the actual plugin and skips a junk `libbroken.dylib` without crashing.
- Observation (M3): The legacy movie QML pages were entirely dead — `MainPage`'s entry button was commented out, and the pages referenced context properties (`movieProcessor`, `movieAlbumModel`), an image provider (`image://movies`), icons (`add_movie.png`, `play_player.png`, …) and Qt 5 modules (`QtQuick.Extras 1.4`, `QtQuick.Controls.Styles 1.4`) that do not exist in this build, so `MoviePage.qml` could never have loaded. Rewrote rather than patched.
  Evidence: pre-change `MoviePage.qml` imports and `MainPage.qml` lines 69–90 (commented button).

## Decision Log

- Decision: Plan authored from analysis of the current tree (post-CMake-migration state on branch `dev_plan_1ststage`).
  Rationale: The qmake-to-CMake ExecPlan (`QMAKE_TO_CMAKE_EXECPLAN.md`) is complete; this plan builds on the CMake build as the only build path and ignores the legacy `.pro` files.
  Date/Author: 2026-06-10 / Claude
- Decision: Keep the camera session in C++ (a `CameraService` QObject exposed to QML) rather than in QML as today.
  Rationale: The plugin system must intercept video frames (`QVideoSink::videoFrameChanged`) and the recorder must share the same `QMediaCaptureSession`; doing this in C++ gives one owner for the session, makes the logic unit-testable, and removes the current duplication where QML and C++ each build half a session.
  Date/Author: 2026-06-10 / Claude
- Decision: Use Qt's native plugin mechanism (`QPluginLoader` + `Q_DECLARE_INTERFACE`) for frame-processor plugins, with frames exchanged as `QImage` (and an optional raw-buffer accessor for zero-copy later).
  Rationale: It is the standard Qt way to load shared libraries with a typed interface, requires no third-party dependency, and keeps OpenCV/deep-learning code strictly inside plugins so the core app never links OpenCV.
  Date/Author: 2026-06-10 / Claude
- Decision: Use Qt Test (`Qt6::Test`) + CTest rather than GoogleTest.
  Rationale: Qt Test ships with the Qt installation already required to build, integrates with signals/slots and `QSignalSpy`, and avoids adding a FetchContent dependency.
  Date/Author: 2026-06-10 / Claude
- Decision: Replace the singleton with constructor injection of `DatabaseManager`, keeping a default path under `QStandardPaths::AppDataLocation`.
  Rationale: Injection is the smallest change that makes models testable against an in-memory SQLite database (`:memory:`), and moving the default path out of the CWD fixes the "different launch directory = different gallery" defect.
  Date/Author: 2026-06-10 / Claude
- Decision: QML is a thin view layer only; all logic, state, and device/IO work lives in Qt/C++.
  Rationale: Maintainer preference (2026-06-10) for performance and maintainability. Concretely: no `CaptureSession`/`Camera` construction in QML, no path or persistence policy in QML, no JavaScript business logic beyond trivial view glue (formatting, visibility bindings). QML files contain only layout, styling, bindings to C++ properties, and calls to C++ invokables. Any milestone implementation that would add logic to a `.qml` file must move it into `CameraService`, a model, or a coordinator object instead.
  Date/Author: 2026-06-10 / maintainer preference, recorded by Claude
- Decision: Icons move to a single SVG set (Material Symbols outlined style, Apache 2.0 licensed), recolored at runtime via `icon.color`.
  Rationale: The current PNGs are mixed-style raster assets that scale poorly on HiDPI; SVG + `icon.color` gives theme-consistent icons from one source per glyph.
  Date/Author: 2026-06-10 / Claude

- Decision: `logger-core` receives the log directory as an `InitLogger(const QString&)` argument instead of knowing about `StorageLocations`.
  Rationale: camera-core links logger-core, so logger-core cannot depend back on camera-core; path policy belongs to the composition root (`main.cpp`).
  Date/Author: 2026-06-10 / Claude
- Decision: The `appPath` QML context property was replaced by a `logsPath` URL context property.
  Rationale: The only consumer was `LoggerPage.qml` computing `appPath + "/logs"` — path policy in QML violates the thin-view rule; C++ now hands QML the finished URL.
  Date/Author: 2026-06-10 / Claude

- Decision: Recordings are presented as one flat "Recordings" list (no movie albums yet); `movies.album_id` exists and is nullable so album grouping can be added without a migration. `MovieAlbumListPage.qml` moved to `attic/`.
  Rationale: The movie-album UI was dead code with no working model behind it; a flat list achieves the milestone's observable outcome (recordings visible and playable) with the least new surface, and the schema keeps the album option open.
  Date/Author: 2026-06-10 / Claude
- Decision: MoviePage player controls use Unicode glyph buttons (▶ ⏸ ⏹ ⏪ ⏩) temporarily.
  Rationale: The referenced player PNGs never existed in resources; Milestone 7 replaces all icons with SVGs anyway, so adding interim raster assets would be churn.
  Date/Author: 2026-06-10 / Claude
- Decision: `CameraService::applyCamera()` stops any active recording before switching devices, and `CameraPage` stops recording on page destruction.
  Rationale: A `QMediaRecorder` cannot survive its source `QCamera` being destroyed; stopping produces a valid finished file instead of a corrupt one.
  Date/Author: 2026-06-10 / Claude

- Decision: Photo capture keeps `CameraService::captureImage()` parameterless; the new `CaptureCoordinator` files the saved image into `PictureModel`'s already-loaded album rather than passing an `albumId` through the camera, as the plan's `captureImage(int albumId)` sketch suggested.
  Rationale: `PictureModel` already owns "the current album" (`setAlbumId`/`getAlbumId`) set when the user enters an album, and the camera is opened globally from `MainPage` with no album in scope. Routing the album id through the camera would duplicate state the model already holds. The coordinator guards `getAlbumId() <= 0` so a capture with no album selected is logged, not written as an orphan row. Observable behavior is unchanged: enter an album, open the camera, capture, and the photo is in that album on return.
  Date/Author: 2026-06-11 / Claude
- Decision: `PictureProvider` became a `QQuickAsyncImageProvider` and the previous in-process `QCache<QString,QPixmap>` was dropped in favor of the on-disk `ThumbnailCache` plus QML's own image cache.
  Rationale: The async provider moves decode/scale off the GUI thread (the plan's anti-stall goal); a persistent disk cache survives restarts and is shared by any consumer, and keeping two caches would be redundant. The row→path resolution stays on the GUI thread inside `requestImageResponse` (where Qt invokes it) so the worker never touches the model.
  Date/Author: 2026-06-11 / Claude
- Decision: The frame tap swaps the capture session's video sink at runtime (`setVideoSink(&m_processingSink)` when processors are enabled vs. `setVideoOutput(m_videoOutput)` when none are) and forwards processed frames to the VideoOutput's sink only in the tapped state.
  Rationale: Satisfies "zero overhead when no plugin is enabled" — the session drives the QML output directly in that case. Frames are processed latest-only (an atomic `m_frameBusy` drops frames while the worker is busy) so the preview never falls behind. `QImageCapture` stays attached to the session, so stills are always captured from the original camera stream, not the processed preview.
  Date/Author: 2026-06-11 / Claude
- Decision: The grayscale plugin returns `Format_RGBA8888` (grayscale-8 converted back to RGBA) rather than `Format_Grayscale8`.
  Rationale: A predictable, widely-supported format for wrapping the result in `QVideoFrame(QImage)` (Qt 6.8+) before pushing to the output sink; avoids per-format special-casing in `CameraService::onFrameProcessed`.
  Date/Author: 2026-06-11 / Claude
- Decision (review fix): `PluginManager` is constructed before `CameraService` in `main.cpp` so it is destroyed after the service.
  Rationale: The service's worker thread executes processor code living inside the loaded plugin libraries; the manager's destructor unloads those libraries. With the original declaration order the manager was destroyed first, leaving a small exit-time window where an in-flight `process()` call could run unmapped code. Reversing construction order makes `CameraService`'s destructor (which joins the worker thread) run before any library is unloaded.
  Date/Author: 2026-06-12 / Claude (post-implementation review)
- Decision (review fix): `FrameProcessingWorker::process()` now takes the `QVideoFrame` itself and performs the `toImage()` conversion on the worker thread, instead of `CameraService::onFrameChanged` converting on the GUI thread.
  Rationale: `QVideoFrame::toImage()` can involve a GPU readback and is the most expensive per-frame step besides the processors; doing it on the GUI thread undercut the worker-thread design. `QVideoFrame` is implicitly shared, so handing it across the queued connection is cheap.
  Date/Author: 2026-06-12 / Claude (post-implementation review)
- Decision (M6): The `PluginManager` and grayscale-processor tests compile the relevant source files directly into the test executable rather than testing through a separately-built library, and `PluginManager` gained an optional `loadPlugins(const QStringList& extraDirs)` parameter pointed at `$<TARGET_FILE_DIR:grayscale-processor>` via a compile define.
  Rationale: `PluginManager` and `GrayscaleProcessor` live in the app/plugin targets, not in `camera-core`, so there is no library to link; compiling the source in is the smallest path to a unit test and AUTOMOC handles the `Q_OBJECT`/`Q_PLUGIN_METADATA` vtables. The `extraDirs` hook mirrors `StorageLocations::setRootForTesting` — production `loadPlugins()` is unchanged, and the test points discovery at the real build output without installing anything. QSettings is isolated by setting a test-only organization/application name so persisted enable-state never touches the real app.
  Date/Author: 2026-06-12 / Claude
- Decision (M7): Icons are original, hand-authored single-color SVGs released CC0, not imported Material Symbols as the plan sketched.
  Rationale: The plan's goal was crisp, theme-tinted, single-source icons; authoring originals achieves that with a clean public-domain provenance and no third-party download/attribution dependency. Recoloring is done at runtime via each control's `icon.color` against `Style.iconColor`/`dangerColor`, so the SVGs' own fill is irrelevant. README records the CC0 note (there was no pre-existing Flaticon attribution in `main.cpp` to replace — it was removed in an earlier milestone).
  Date/Author: 2026-06-12 / Claude
- Decision (M7): A single `delete` glyph and a single `rename` glyph serve both the album and photo/movie buttons, and no Windows/macOS application-window icon was added.
  Rationale: The album/photo delete (and rename) actions are the same concept, so one glyph each keeps the set minimal while covering every prior PNG use. `app_resource.rc` carries only version info (no icon reference) and no `.icns` art asset exists, so the application-bundle icon is left out of scope as the plan permitted.
  Date/Author: 2026-06-12 / Claude

## Outcomes & Retrospective

- (2026-06-12) Milestones 6–7 complete; the full plan (M1–M7) is now delivered. A six-executable Qt Test suite runs under CTest (`ctest --output-on-failure` → 6/6 passing) covering storage paths, the de-singletoned database, the album/picture models (including the album→pictures cascade), plugin discovery/IID-validation/enable-persistence, and the grayscale processor's pixel output — exactly the harness checks from M4–M5 promoted into the build. The UI now uses a single set of theme-tinted SVG icons; the raster PNGs are gone from the tree (kept in history). The earlier post-implementation review fixes (plugin/service destruction order, off-GUI-thread frame conversion) are in place. Remaining work is interactive-only and out of this plan's automatable scope: live camera preview, device/format switching, end-to-end record→playback, the photo-into-album round-trip, and live grayscale preview all need a human session with camera permission granted. The macOS/Windows application-bundle icon art was intentionally left out of scope.
- (2026-06-11) Milestones 4–5 complete. Captured stills are registered into the open album live via `CaptureCoordinator`; album thumbnails are produced once on disk by `ThumbnailCache` and served asynchronously by a rewritten `PictureProvider`, so large albums no longer stall the GUI. A runtime plugin system (`processor-api` interface, `PluginManager`, a `grayscale-processor` proof plugin) is in place with a per-frame worker-thread tap in `CameraService` and a "Processors" toggle list in the camera drawer; the core app links no OpenCV. Verified by clean build, app startup with the plugin discovered/registered, and a standalone harness exercising `GrayscaleProcessor::process()` and `ThumbnailCache`. Remaining gaps by design: live grayscale preview and the photo-into-album round-trip need an interactive session with camera permission; Milestone 6 (tests) will formalize the harness checks into CTest, and Milestone 7 (icons) is still pending.
- (2026-06-10) Milestones 1–2 complete. The persistence layer is injection-based and test-ready (named SQLite connections, in-memory databases possible), all storage lives under the platform app-data root independent of CWD, and the capture pipeline has a single C++ owner with device/format selection surfaced in a QML drawer. QML pages contain no camera or path logic. Remaining gap: interactive verification of device/format switching needs a human session with camera permission; recording (M3) and event-driven gallery registration (M4) are unchanged stubs by design.

## Context and Orientation

The repository root contains `QML_ML_Camera/`, the entire application. It builds with CMake (root list file `QML_ML_Camera/CMakeLists.txt`) into three targets:

- `logger-core` (`QML_ML_Camera/logger-core/`): a shared library installing a Qt message handler that writes CSV logs into a `logs/` folder. Key files: `logger.h/.cpp`, `loggingcategories.h/.cpp` defining categories used as `qDebug(logInfo()) << ...`.
- `camera-core` (`QML_ML_Camera/camera-core/`): a shared library with the persistence layer. `DatabaseManager` (singleton) opens SQLite and owns three DAOs (`AlbumDAO`, `PictureDAO`, `LoggerDAO`) that issue raw SQL. `AlbumModel`, `PictureModel`, `LoggerModel` are `QAbstractListModel` subclasses that QML binds to. "DAO" means Data Access Object: a small class whose only job is running SQL for one table.
- `QML_ML_Camera_App` (`QML_ML_Camera/app/`): the executable. `main.cpp` creates the models, registers them as QML context properties, registers `CameraProcessor` as QML type `solid.broccoli 1.0`, installs `PictureProvider` (a `QQuickImageProvider` serving `image://pictures/<id>` thumbnails), and loads `qrc:/main.qml`. The QML UI is a `StackView` of pages (`MainPage`, `CameraPage`, `AlbumListPage`, `AlbumPage`, `PicturePage`, movie equivalents, `LoggerPage`) with shared style singletons (`Style.qml`, `PageTheme.qml`, `ToolBarTheme.qml`).

Build commands (working directory: repository root):

    cmake -S QML_ML_Camera -B QML_ML_Camera/build
    cmake --build QML_ML_Camera/build

Known defects this plan fixes are listed in `Surprises & Discoveries` above. The legacy qmake files (`*.pro`, checked-in `Makefile*`) are not maintained by this plan; Milestone 1 deletes the checked-in generated Makefiles.

## Plan of Work

### Milestone 1 — Core architecture restructure

Goal: `camera-core` becomes a clean, testable layer; the app composes objects explicitly; generated build files leave the tree.

Work, in order:

1. Delete checked-in generated files: `QML_ML_Camera/**/Makefile`, `Makefile.Debug`, `Makefile.Release`, `.qmake.stash`, `QML_ML_Camera.pro.user`, and stray `.DS_Store` files. Add `build/`, `*.user`, `.DS_Store`, `logs/`, `*.db` to `.gitignore` if not already present. Do not delete the `.pro` files yet (maintainer decision pending from the previous plan).
2. De-singleton `DatabaseManager`. In `QML_ML_Camera/camera-core/DatabaseManager.h`: make the constructor public, taking an explicit database path; remove `instance()`; add `bool isOpen() const`. Default path helper: a static `QString DatabaseManager::defaultDatabasePath()` returning `QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/solidBroccoli_Gallery.db"`, creating the directory if missing. Use a unique connection name per instance (`QSqlDatabase::addDatabase("QSQLITE", connectionName)`) so tests can open several databases in one process.
3. Inject the manager: `AlbumModel`, `PictureModel`, `LoggerModel` take `DatabaseManager&` in their constructors instead of calling `DatabaseManager::instance()`. `main.cpp` constructs one `DatabaseManager db(DatabaseManager::defaultDatabasePath());` and passes it down.
4. Move `MovieProcessor.*`, `MovieAlbum*` either into the build or out of the tree. Decision: move them to a new `QML_ML_Camera/attic/` directory (not compiled) so Milestone 3 can mine them for the movie-album model without shipping dead code; record what is reused in the Decision Log when Milestone 3 executes.
5. Centralize storage paths in a new `camera-core` class `StorageLocations` (static functions `picturesDir()`, `recordingsDir()`, `thumbnailsDir()`, all under `QStandardPaths::AppDataLocation` or, on desktop, `QStandardPaths::PicturesLocation`/`MoviesLocation` subfolder `SolidBroccoli`). Replace every `QDir::currentPath()`-relative path in `CameraProcessor`, `logger.cpp`, and QML with these.

Acceptance: project configures and builds; running the app creates its database and folders under the standard app-data location; launching from any working directory shows the same gallery; `git status` shows no generated files tracked.

### Milestone 2 — Camera service and setup UI

Goal: one C++ owner of the capture pipeline, with device and format selection surfaced in the UI.

1. Create `QML_ML_Camera/app/CameraService.h/.cpp`, a `QObject` replacing `CameraProcessor` (delete `CameraProcessor.*` at the end of this milestone). It owns `QCamera`, `QMediaCaptureSession`, `QImageCapture`, `QMediaRecorder`, and a `QVideoSink` used as a frame tap. Q_PROPERTYs: `videoSink` (the QML `VideoOutput.videoSink` is assigned to the session output), `availableCameras` (list model built from `QMediaDevices::videoInputs()`), `currentCameraIndex`, `availableFormats` (resolution + pixel format + max FPS strings for the current device, from `QCameraDevice::videoFormats()`), `currentFormatIndex`, `active`. Register it with `qmlRegisterSingletonInstance("solid.broccoli", 1, 0, "CameraService", service)` so every page sees the same instance; construct it in `main.cpp`.
2. React to hot-plug: connect `QMediaDevices::videoInputsChanged` to rebuild `availableCameras` and fall back to the default device if the current one disappears.
3. Rewrite `QML_ML_Camera/app/CameraPage.qml`: remove the QML `CaptureSession`/`Camera`/`ImageCapture` block entirely; keep `VideoOutput` and bind `CameraService.videoSink = videoOutput.videoSink` (or assign session output in C++ via an invokable `attachVideoOutput(QObject*)` — choose the invokable, it keeps the QML one-line). Add a settings drawer (QML `Drawer` opened by the existing cogwheel icon) with two `ComboBox`es bound to `availableCameras`/`availableFormats` and the existing `SettingSwitcher` style.

Acceptance: app shows live preview through the C++ session; switching device or format in the drawer visibly changes the preview; unplugging a USB camera while previewing falls back to the default device without crashing (log line records the fallback).

### Milestone 3 — Video recording

1. In `CameraService`, implement `Q_INVOKABLE void startRecording()` / `stopRecording()` using the owned `QMediaRecorder` attached to the session. Output file: `StorageLocations::recordingsDir() + "/SolidBroccoli_VID_" + yyyy-MM-dd_hh-mm-ss + ".mp4"`, `QMediaFormat::MPEG4`/`H264` with fallback to the platform default if unsupported (`QMediaFormat::isSupported`). Expose `recording` (bool, from `QMediaRecorder::recorderStateChanged`) and `recordingDuration` properties for the UI.
2. Persistence: create `MovieDAO`/`MovieModel` in `camera-core` mirroring the Picture pair (table `movies`: id, album id nullable, file path, name, duration, created-at). On `QMediaRecorder::recorderStateChanged` to Stopped with no error, insert the row. The existing `Movie.h` struct in `camera-core` is the starting point.
3. UI: on `CameraPage.qml`, the record `RoundButton` (icon `record`) toggles start/stop, turns red and pulses while `CameraService.recording`, and shows `recordingDuration` as mm:ss text. `MovieAlbumPage.qml`/`MoviePage.qml` list and play recordings via QML `MediaPlayer` + `VideoOutput`, backed by `MovieModel` registered as context property `movieModel`.

Acceptance: pressing record then stop produces a playable `.mp4` in the recordings folder, a new row in the `movies` table, and the clip appears and plays inside the app's movie pages.

### Milestone 4 — Image capture and saving pipeline

1. Capture flow moves fully into `CameraService`: `Q_INVOKABLE void captureImage(int albumId)` calls `QImageCapture::captureToFile(StorageLocations::picturesDir() + "/SolidBroccoli_PIC_<timestamp>.jpg")`. Connect `QImageCapture::imageSaved(int id, const QString& fileName)` to emit `imageSaved(QString filePath)`; connect `errorOccurred` to emit `captureError(QString message)` surfaced as a QML toast/label.
2. Database registration becomes event-driven: a small `CaptureCoordinator` QObject (app layer) listens to `CameraService::imageSaved` and calls `PictureModel::addPictureFromUrl()` for the current album, so the gallery model updates immediately (today the user must navigate for a reload). Shutter feedback: flash overlay rectangle animated on `imageSaved`.
3. Thumbnails: `PictureProvider` currently scales full images on demand on the GUI thread. Add a `ThumbnailCache` in `camera-core`: on first request, generate a 400-px JPEG under `StorageLocations::thumbnailsDir()` on a `QThreadPool` task; `PictureProvider` switches to `QQuickAsyncImageProvider` returning the cached file. This removes UI stalls when opening large albums.

Acceptance: capturing a photo while inside an album shows the new photo in that album within a second without navigation; capture errors (e.g., camera busy) show a visible message instead of silently logging; scrolling an album of 50+ photos does not freeze the UI.

### Milestone 5 — Frame-processor plugin system (OpenCV / deep-learning ready)

A "frame-processor plugin" is a shared library, loaded at runtime, that receives each preview video frame and may return a modified image; this is the extension point where OpenCV filters or neural-network inference will live, without the application itself linking those libraries.

1. New target `processor-api` (INTERFACE library, headers only) in `QML_ML_Camera/processor-api/`: 

       // FrameProcessor.h
       class FrameProcessor {
       public:
           virtual ~FrameProcessor() = default;
           virtual QString name() const = 0;
           virtual QString description() const = 0;
           virtual bool initialize(QString* errorMessage) = 0;   // load models, allocate
           virtual QImage process(const QImage& frame) = 0;       // called per frame, worker thread
       };
       #define FrameProcessor_iid "broccoli.FrameProcessor/1.0"
       Q_DECLARE_INTERFACE(FrameProcessor, FrameProcessor_iid)

   The interface deliberately uses `QImage` only; an OpenCV plugin converts to `cv::Mat` internally (`cv::Mat(img.height(), img.width(), CV_8UC4, img.bits(), img.bytesPerLine())`). Document this conversion in the header so plugin authors need no other reference.
2. `PluginManager` (app layer): scans `<application dir>/plugins/` plus `StorageLocations` app-data `plugins/` with `QPluginLoader`, validates the IID, exposes a `QAbstractListModel` of plugins (name, description, enabled) for QML, and persists enabled state in `QSettings`.
3. Frame tap: in `CameraService`, when at least one plugin is enabled, connect the session's `QVideoSink::videoFrameChanged`; map each `QVideoFrame` to `QImage` (`frame.toImage()`), run the enabled processors in order on a dedicated worker thread (skip frames if the worker is busy — process latest only, never queue), and push the result to the QML `VideoOutput`'s sink as a new `QVideoFrame`. When no plugin is enabled, the session writes directly to the output sink with zero overhead.
4. Proof plugin `grayscale-processor` in `QML_ML_Camera/plugins/grayscale/`: a `QObject` implementing the interface via `Q_PLUGIN_METADATA(IID FrameProcessor_iid)`, returning `frame.convertToFormat(QImage::Format_Grayscale8)`. Built as MODULE library, output directory set to `<build>/app/plugins/`. This plugin proves the contract that a future `opencv-canny` or `yolo-detector` plugin will follow; those are explicitly out of scope here, but the plan requires that adding them touches only a new plugin directory.
5. UI: a "Processors" section in the camera settings drawer listing plugins with switches.

Acceptance: with the grayscale plugin present and enabled, the live preview turns grayscale and captured images remain unprocessed originals (capture taps the camera, not the processed stream — state this in the UI help text); removing the plugin file and restarting shows an empty processor list; the app builds and runs with zero OpenCV dependency.

### Milestone 6 — Unit testing module

1. Root `CMakeLists.txt`: add `enable_testing()` and `add_subdirectory(tests)` guarded by `BUILD_TESTING` (default ON via `include(CTest)`).
2. `QML_ML_Camera/tests/` with one Qt Test executable per area, each registered via `add_test`:
   - `tst_databasemanager`: opens `:memory:`, asserts tables exist, `isOpen()` true; bad path reports closed.
   - `tst_albummodel` / `tst_picturemodel`: construct models over an in-memory `DatabaseManager`; verify `rowCount`, `addAlbumFromName`, `rename`, `removeRows` (with `QSignalSpy` on `rowsInserted`/`rowsRemoved`), and that pictures cascade-delete with their album.
   - `tst_storagelocations`: paths are absolute and creatable (use `QTemporaryDir` by overriding the base path — give `StorageLocations` a `setRootForTesting(QString)` hook).
   - `tst_pluginmanager`: loads the built grayscale plugin from the build tree, asserts name/IID; a deliberately broken dummy library is skipped without crash.
   - `tst_frameprocessor_grayscale`: feeds a colored `QImage`, asserts output format/pixels.
   This milestone requires the Milestone 1 injection work; nothing here may touch the real app-data database.
3. Make `camera-core` test-friendly: any logic currently needing a `QGuiApplication` must run under `QTEST_GUILESS_MAIN` where possible (`QTEST_MAIN` only for tests needing GUI).

Acceptance (working directory `QML_ML_Camera/build`):

    ctest --output-on-failure

prints all tests passing (expect ≥ 5 test executables, 0 failures). Each new milestone after this one adds or updates tests in the same run.

### Milestone 7 — Modern icons

1. Add `QML_ML_Camera/app/images/svg/` with Material Symbols (outlined) SVGs covering every current PNG use: add-album, add-photo, gallery, back, camera, settings(cogwheel), delete-album, delete-photo, play/forward, help, home, log, photo, quit, record, rename ×2, save. Each file is a single-color path so `icon.color` works.
2. Register them in `app/CMakeLists.txt` resources; switch every `icon.source: "qrc:/images/png/X.png"` in QML to the SVG path and set `icon.color: Style.iconColor` (add `iconColor` and a `dangerColor` for record/delete to `Style.qml`). Set sensible `icon.width/height` from `Style` instead of per-button arithmetic.
3. Replace the Windows resource icon reference in `app_resource.rc` and add a proper `.icns`/`MACOSX_BUNDLE_ICON_FILE` for macOS if an app icon asset is produced; otherwise note it as out of scope in the Decision Log.
4. Remove the now-unused PNGs from resources (keep files in git history only).

Acceptance: every toolbar/button icon renders crisply at 2× DPI, tinted by theme color; no `qrc:/images/png` references remain (`grep -r "images/png" QML_ML_Camera/app/*.qml` returns nothing); the licenses note (Apache 2.0, Material Symbols) is added to `README.md`, replacing the Flaticon attribution comment in `main.cpp`.

## Concrete Steps

All commands run from the repository root unless stated.

Configure and build (after every milestone):

    cmake -S QML_ML_Camera -B QML_ML_Camera/build
    cmake --build QML_ML_Camera/build -j

Run the app:

    ./QML_ML_Camera/build/app/QML_ML_Camera_App.app/Contents/MacOS/QML_ML_Camera_App

Run tests (Milestone 6 onward), from `QML_ML_Camera/build`:

    ctest --output-on-failure

Suggested order of execution is the milestone order; Milestone 6 may begin in parallel as soon as Milestone 1 lands, and Milestone 7 is independent of 2–6.

## Validation and Acceptance

Overall acceptance is behavioral and cumulative: from a clean checkout, configure and build with the commands above; `ctest` passes; launching the app yields a live camera preview; the settings drawer switches devices and formats; record/stop produces a playable mp4 visible in the movie album; photo capture lands in the open album within a second with a visible shutter flash; enabling the grayscale plugin turns the preview gray; all icons are SVG and theme-tinted. Each milestone's own acceptance paragraph above is the gate for marking its checkbox done in `Progress`.

## Idempotence and Recovery

All steps are additive file edits plus deletions of generated/dead files and are safe to re-run; CMake reconfiguration is idempotent. The database schema gains one table (`movies`); guard its creation with `CREATE TABLE IF NOT EXISTS` so re-running against an existing gallery is safe. The relocation of the database to app-data (Milestone 1) abandons any old CWD-relative `solidBroccoli_Gallery.db`; if user data matters, copy the old file to `DatabaseManager::defaultDatabasePath()` once — document this in the release note. Work commits at every milestone boundary on branch `dev_plan_1ststage` (or follow-up branches), so any milestone can be reverted independently.

## Interfaces and Dependencies

- Qt 6.5+ (local toolchain is 6.11): Core, Qml, Quick, Sql, Svg, Multimedia, Test. No new third-party dependency in the core app.
- `QML_ML_Camera/processor-api/FrameProcessor.h` defines the plugin contract exactly as written in Milestone 5; plugins additionally `#include <QtPlugin>` and use `Q_PLUGIN_METADATA(IID FrameProcessor_iid)`.
- `camera-core` public surface after Milestone 1: `DatabaseManager(const QString& path)`, `DatabaseManager::defaultDatabasePath()`, `StorageLocations::{picturesDir,recordingsDir,thumbnailsDir,setRootForTesting}`, model constructors taking `DatabaseManager&`.
- `app` QML-facing surface: singleton `CameraService` (`solid.broccoli 1.0`) with properties/invokables listed in Milestones 2–4, context properties `albumModel`, `pictureModel`, `movieModel`, `loggerModel`, image providers `pictures` (async) and movie thumbnails.
- OpenCV and any DL runtime (ONNX Runtime, LibTorch, etc.) are plugin-side dependencies only and are out of scope for this plan beyond the interface contract.
