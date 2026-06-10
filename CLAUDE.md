# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**solid-broccoli** is a Qt6/QML camera application with photo and video album management. It captures images and video from a USB/system camera, stores metadata in SQLite, and displays a QML-based gallery UI. There is no OpenCV dependency in the current codebase despite the README mention — the current implementation uses Qt Multimedia only.

## Build

The project uses CMake (migrated from qmake). The build directory is `QML_ML_Camera/build/`.

```bash
# Configure (from repo root)
cmake -S QML_ML_Camera -B QML_ML_Camera/build

# Build
cmake --build QML_ML_Camera/build

# Run
./QML_ML_Camera/build/app/QML_ML_Camera_App.app/Contents/MacOS/QML_ML_Camera_App
```

**Dependencies:** Qt6 (Core, Qml, Quick, Sql, Svg, Multimedia). On macOS with Homebrew Qt: ensure `Qt6_DIR` or `CMAKE_PREFIX_PATH` points to the Qt installation if CMake cannot find it automatically.

The legacy qmake files (`*.pro`, `Makefile.*`) remain in the tree but are not the active build system.

## Architecture

The project is split into three CMake targets:

| Target | Directory | Role |
|--------|-----------|------|
| `logger-core` (shared lib) | `QML_ML_Camera/logger-core/` | File-based CSV logger using Qt's `qInstallMessageHandler`. Wraps `QLoggingCategory` macros (`logInfo()`, etc.). |
| `camera-core` (shared lib) | `QML_ML_Camera/camera-core/` | Domain models, DAOs, and SQLite persistence via `DatabaseManager` singleton. |
| `QML_ML_Camera_App` (executable) | `QML_ML_Camera/app/` | Qt Quick application, QML UI, `CameraProcessor` QObject, `PictureProvider` image provider. |

### camera-core layer

`DatabaseManager` is a singleton that opens `solidBroccoli_Gallery.db` (SQLite) in the current working directory and owns three DAO instances: `AlbumDAO`, `PictureDAO`, `LoggerDAO`. Each DAO provides raw SQL operations; the corresponding `*Model` classes (e.g. `AlbumModel`, `PictureModel`) inherit `QAbstractListModel` and serve as the bridge between the database and QML.

`PictureModel` is constructed with a reference to `AlbumModel` because pictures are scoped to albums.

### app layer

`main.cpp` registers context properties (`albumModel`, `pictureModel`, `loggerModel`) and an image provider (`pictures`) on the QML engine before loading `qrc:/main.qml`. `CameraProcessor` is registered as a QML type (`solid.broccoli 1.0`).

`CameraProcessor` wraps `QCamera`, `QMediaCaptureSession`, `QImageCapture`, and `QMediaRecorder`. It is instantiated in QML (not in `main.cpp`) because it is a registered QML type.

`PictureProvider` implements `QQuickImageProvider` and serves thumbnail images to QML via `image://pictures/<id>`.

### QML UI structure

`main.qml` is a `StackView`-based navigator. Pages pushed onto the stack:
- `MainPage` — home with navigation buttons
- `CameraPage` — live camera viewfinder + capture/record controls using `CameraProcessor`
- `AlbumListPage` / `AlbumPage` / `PicturePage` — photo album browsing
- `MovieAlbumListPage` / `MovieAlbumPage` / `MoviePage` — video album browsing
- `LoggerPage` — displays log entries from `loggerModel`

`Style.qml`, `PageTheme.qml`, `ToolBarTheme.qml` provide shared visual constants. `SettingSwitcher.qml` and `InputDialog.qml` are reusable UI components.

## ExecPlans (PLANS.md)

Feature work in this repo uses "ExecPlan" design documents defined by `PLANS.md`. When implementing a non-trivial feature, read `PLANS.md` in full and author a self-contained ExecPlan markdown file before writing code. ExecPlans are living documents: update `Progress`, `Decision Log`, and `Surprises & Discoveries` as work proceeds.

## Logging

Logs are written to `logs/SolidBroccoli_Log_<date>.csv` relative to the working directory. Use `logInfo()`, `logWarning()`, `logCritical()` (from `loggingcategories.h`) with `qDebug()`/`qWarning()` — do not use plain `qDebug()` without a category.
