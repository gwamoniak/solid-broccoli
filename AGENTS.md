# Repository Guidelines

## Project Structure & Module Organization

This repository contains a Qt/QML camera application under `QML_ML_Camera/`.
The CMake project builds three modules:

- `logger-core/`: shared logging library (`Logger`, logging categories).
- `camera-core/`: shared data/model layer for albums, pictures, movies, DAOs, and database access.
- `app/`: QML application, C++ processors, image provider, resources, icons, and UI files.

QML views live directly in `QML_ML_Camera/app/`. Image assets are in
`QML_ML_Camera/app/images/png/`. Generated build artifacts such as `Makefile`,
`Makefile.Debug`, and `Makefile.Release` should not be edited by hand.

## Build, Test, and Development Commands

Primary build system is CMake:

```sh
cmake -S QML_ML_Camera -B QML_ML_Camera/build
cmake --build QML_ML_Camera/build
```

This configures an out-of-source build and builds `logger-core`, `camera-core`,
then `QML_ML_Camera_App`. To run a headless smoke check:

```sh
QML_ML_Camera/build/app/QML_ML_Camera_App -platform offscreen
```

Legacy qmake files remain for comparison during migration:

```sh
cd QML_ML_Camera
qmake QML_ML_Camera.pro
make
```

## Coding Style & Naming Conventions

Use C++20 and Qt 6 APIs for new C++ work. Follow existing Qt conventions: `PascalCase` for classes,
`camelCase` for methods, and `m_` prefixes for private members. Keep QML
component filenames in `PascalCase.qml`, matching the component name.

Prefer Qt types and APIs (`QString`, `QDir`, `QUrl`, signals/slots) for Qt-facing
code. Keep comments short and only where they clarify non-obvious behavior.

## Testing Guidelines

There is no test suite currently checked in. For new logic in `camera-core` or
`logger-core`, add focused Qt Test coverage when practical, using filenames such
as `tst_PictureModel.cpp` or `tst_Logger.cpp`. Manual verification should include
launching the app, opening album/gallery pages, capturing an image, and checking
that log output and saved files are created in the expected directories.

## Commit & Pull Request Guidelines

Existing history uses short messages such as `Update README.md` and `fixing icons`.
Keep commits concise and imperative, for example `Update camera capture path` or
`Add CMake build files`.

Pull requests should describe the changed module, user-visible behavior, and
build/test steps performed. Include screenshots or screen recordings for QML UI
changes, and call out Qt version, platform, and camera hardware used for camera
or multimedia changes.

## Agent-Specific Instructions

Do not revert unrelated user changes. Keep edits scoped to source, QML, resource,
or build files needed for the requested task. Keep generated build and runtime
outputs out of version control.

# ExecPlans

When writing complex features or significant refactors, use an ExecPlan (as described in PLANS.md) from design to implementation.
