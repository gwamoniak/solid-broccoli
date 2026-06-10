# Move QML_ML_Camera from qmake to CMake

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds.

This document follows the repository guidance in `PLANS.md`. It is self-contained so a contributor can complete the qmake-to-CMake migration from the current working tree without prior context.

## Purpose / Big Picture

The project currently builds with qmake through `QML_ML_Camera/QML_ML_Camera.pro`, which coordinates the `logger-core`, `camera-core`, and `app` subprojects. This migration adds an out-of-source CMake build that preserves the same module boundaries and build order. After the change, a contributor can configure and build the whole application with `cmake -S QML_ML_Camera -B QML_ML_Camera/build` and `cmake --build QML_ML_Camera/build`.

The first successful outcome is build-system parity: CMake builds the same libraries and executable that qmake builds today. Qt 6 API migration is intentionally separate work; this plan may prepare for Qt 6, but it should not mix in camera API rewrites unless required to compile with the selected Qt version.

## Progress

- [x] (2026-05-03T18:59:30Z) Read the existing qmake structure and confirmed the project has `logger-core`, `camera-core`, and `app` subprojects.
- [x] (2026-05-03T18:59:30Z) Created this implementation plan.
- [x] (2026-05-03T18:59:30Z) Add root `QML_ML_Camera/CMakeLists.txt`.
- [x] (2026-05-03T18:59:30Z) Add `QML_ML_Camera/logger-core/CMakeLists.txt`.
- [x] (2026-05-03T18:59:30Z) Add `QML_ML_Camera/camera-core/CMakeLists.txt`.
- [x] (2026-05-03T18:59:30Z) Add `QML_ML_Camera/app/CMakeLists.txt`.
- [x] (2026-05-03T18:59:30Z) Configure the CMake build and fix target, include, resource, or linkage errors.
- [x] (2026-05-03T18:59:30Z) Build the project with CMake and record the successful command output.
- [x] (2026-05-03T18:59:30Z) Keep qmake files temporarily after parity so existing workflows are not removed in the same change.

## Surprises & Discoveries

- Observation: No test files or test directories were found under `QML_ML_Camera`.
  Evidence: `find QML_ML_Camera -maxdepth 3 -type f -name '*test*' -o -name '*Test*'` produced no output.
- Observation: `app/qml.qrc` embeds `main.cpp`, `PictureProvider.cpp`, and `PictureProvider.h` as resources.
  Evidence: these files are listed inside `QML_ML_Camera/app/qml.qrc`. CMake migration should avoid carrying source files into runtime resources unless there is a proven runtime need.
- Observation: The local qmake installation reports Qt 6.11.0, so the CMake migration exposed Qt 6 compile errors rather than Qt 5 compatibility issues.
  Evidence: `qmake -v` printed `Using Qt version 6.11.0 in /opt/homebrew/lib`.
- Observation: Qt 6 no longer accepts the old logger and camera APIs unchanged.
  Evidence: the first CMake build failed on unqualified `endl`; source inspection also found `QCameraImageCapture`, `QCameraInfo`, `QCameraFocus`, and `QRegExp`, which are not appropriate for the Qt 6 build.

## Decision Log

- Decision: Add CMake support before deleting qmake files.
  Rationale: Keeping qmake during the first milestone provides a known working fallback and makes parity easier to verify.
  Date/Author: 2026-05-03 / Codex

- Decision: Preserve the existing three-module shape in CMake.
  Rationale: The qmake root project already expresses the correct dependency order: `logger-core`, then `camera-core`, then `app`.
  Date/Author: 2026-05-03 / Codex

- Decision: Treat Qt 6 source API migration as separate from CMake migration.
  Rationale: `CameraProcessor` uses Qt 5 Multimedia classes such as `QCameraImageCapture` and `QCameraInfo`; replacing those changes application behavior and should be validated independently from build-system migration.
  Date/Author: 2026-05-03 / Codex

- Decision: Make the minimal Qt 6 source edits required for CMake to compile with the local Qt 6.11 toolchain.
  Rationale: The local environment does not provide a Qt 5 build path through qmake; CMake parity had to compile against Qt 6. The implementation replaced `QCameraImageCapture` with `QImageCapture`, introduced `QMediaCaptureSession`, replaced `QRegExp` usage with `QRegularExpression`, and changed `endl` to `Qt::endl`.
  Date/Author: 2026-05-03 / Codex

- Decision: Keep `.pro` files for now.
  Rationale: CMake now builds successfully, but removing qmake files is a workflow decision that should happen after maintainers accept CMake as the primary build path.
  Date/Author: 2026-05-03 / Codex

## Outcomes & Retrospective

Implementation is complete for the build-system migration. CMake now configures and builds `logger-core`, `camera-core`, and `QML_ML_Camera_App` from `QML_ML_Camera/build/`. A lightweight offscreen smoke run did not print QML load errors before the process session ended. qmake files remain as legacy fallback inputs.

## Context and Orientation

The repository root contains `QML_ML_Camera/`, `AGENTS.md`, and `PLANS.md`. The application source is under `QML_ML_Camera/`.

`qmake` is Qt's older project generator. It reads `.pro` files and generates platform-specific Makefiles. `CMake` is the replacement build generator to add here. An out-of-source build means generated files go into a separate directory such as `QML_ML_Camera/build/` instead of mixing with source files.

The current qmake files are:

- `QML_ML_Camera/QML_ML_Camera.pro`: subdirs project. It builds `logger-core`, `camera-core`, then `app`.
- `QML_ML_Camera/logger-core/logger-core.pro`: shared library using Qt Core only.
- `QML_ML_Camera/camera-core/camera-core.pro`: shared library using Qt Sql and linking `logger-core`.
- `QML_ML_Camera/app/app.pro`: application using Qt Qml, Quick, Sql, Svg, Multimedia, resources from `qml.qrc`, and linking both libraries.

The important source locations are:

- `QML_ML_Camera/logger-core/`: `logger.cpp`, `loggingcategories.cpp`, headers, and export macro header.
- `QML_ML_Camera/camera-core/`: album, picture, logger model, DAO, and database classes.
- `QML_ML_Camera/app/`: `main.cpp`, `CameraProcessor.cpp`, `PictureProvider.cpp`, QML files, `qml.qrc`, and image assets under `images/png/`.

## Plan of Work

First, add `QML_ML_Camera/CMakeLists.txt` with the project declaration, C++ standard, Qt package lookup, and subdirectories. The implemented project uses `find_package(Qt6 REQUIRED COMPONENTS Core Qml Quick Sql Svg Multimedia)` because the available local Qt version is Qt 6.11.

Second, add `logger-core/CMakeLists.txt`. Define a shared library named `logger-core`, compile `logger.cpp` and `loggingcategories.cpp`, link `Qt::Core` or `Qt6::Core`, add `LOGGERCORE_LIBRARY` privately, and expose the module directory with `target_include_directories(logger-core PUBLIC ...)`.

Third, add `camera-core/CMakeLists.txt`. Define a shared library named `camera-core` using the same source list present in `camera-core.pro`. Link Qt Core, Qt Sql, and `logger-core`. Add `CAMERACORE_LIBRARY` privately and expose the module include directory publicly. Do not add movie-related files yet, because they exist in the tree but are not currently part of the qmake target.

Fourth, add `app/CMakeLists.txt`. Define an executable from `main.cpp`, `CameraProcessor.cpp`, and `PictureProvider.cpp`. Link Qt Core, Qml, Quick, Sql, Svg, Multimedia, `camera-core`, and `logger-core`. Add `APP_VERSION` with the project version. Add QML and PNG assets through `qt_add_resources` or, if supporting Qt 5 during transition, `qt5_add_resources`. Include the same runtime QML and image files listed by `app/qml.qrc`, but exclude C++ source and header files from the resource list unless a runtime check proves they are required.

Fifth, configure and build from the repository root. Missing includes or link libraries were handled with target dependencies. Qt 6 API issues were fixed minimally in `logger-core/logger.cpp`, `app/CameraProcessor.h`, and `app/CameraProcessor.cpp`.

Finally, once CMake builds successfully, update `AGENTS.md` so CMake is the primary build command and qmake is described as legacy. The `.pro` files were kept.

## Concrete Steps

Run commands from the repository root, `/Users/gwamoniak/Desktop/sandbox/cpp/solid-broccoli`.

Create the root CMake file:

    $EDITOR QML_ML_Camera/CMakeLists.txt

Create module CMake files:

    $EDITOR QML_ML_Camera/logger-core/CMakeLists.txt
    $EDITOR QML_ML_Camera/camera-core/CMakeLists.txt
    $EDITOR QML_ML_Camera/app/CMakeLists.txt

Configure the build:

    cmake -S QML_ML_Camera -B QML_ML_Camera/build

If Qt is installed outside CMake's default search paths, pass its prefix:

    cmake -S QML_ML_Camera -B QML_ML_Camera/build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.x.x/macos

Build:

    cmake --build QML_ML_Camera/build

Observed successful build ending:

    [100%] Linking CXX executable QML_ML_Camera_App
    [100%] Built target QML_ML_Camera_App

Run a headless smoke check:

    QML_ML_Camera/build/app/QML_ML_Camera_App -platform offscreen

This command stayed alive briefly without printing QML load errors, which indicates the app executable started and loaded its initial QML resources.

For comparison during the migration, the legacy qmake build is:

    cd QML_ML_Camera
    qmake QML_ML_Camera.pro
    make

## Validation and Acceptance

The CMake migration is accepted because `cmake -S QML_ML_Camera -B QML_ML_Camera/build` configured successfully and `cmake --build QML_ML_Camera/build` built the libraries and app executable without qmake.

The build output should show targets equivalent to `logger-core`, `camera-core`, and the app executable. If CMake uses Unix Makefiles, a successful end state should end with progress reaching 100 percent and no compiler or linker errors.

The executable was launched with `-platform offscreen` from the build output location. A full interactive camera/UI verification still requires a desktop session and camera access.

There is no automated test suite today. Do not claim test coverage was added unless a new Qt Test target is introduced.

## Idempotence and Recovery

The CMake build directory is disposable. If configuration becomes inconsistent, delete only `QML_ML_Camera/build/` and rerun the configure command. Do not delete source files, `.pro` files, or user-created artifacts as part of recovery.

Adding CMake files is safe and additive. Keep qmake files until CMake parity is demonstrated. If a CMake experiment fails, leave notes in `Surprises & Discoveries`, revert only the CMake changes made for that experiment, and preserve unrelated user edits.

## Artifacts and Notes

Expected target dependency graph:

    logger-core
      -> camera-core
           -> QML_ML_Camera_App

Minimum CMake version should be at least 3.16 for basic Qt CMake support. Prefer 3.21 or newer when using modern Qt 6 helper functions such as `qt_standard_project_setup`.

Do not use global commands such as `include_directories()` or `link_libraries()` unless unavoidable. Prefer target-scoped commands so dependencies stay local and understandable:

    target_link_libraries(camera-core PUBLIC Qt6::Core Qt6::Sql logger-core)
    target_include_directories(camera-core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

## Interfaces and Dependencies

The root CMake project must provide these targets:

- `logger-core`: shared library built from `QML_ML_Camera/logger-core`.
- `camera-core`: shared library built from `QML_ML_Camera/camera-core` and linked to `logger-core`.
- `QML_ML_Camera_App`: executable built from `QML_ML_Camera/app` and linked to both shared libraries.

The root project must require C++20 for new work:

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

The Qt dependencies needed by the current source are Core, Qml, Quick, Sql, Svg, and Multimedia. OpenCV settings exist in `app/app.pro`, but the currently built C++ files do not show active OpenCV usage. Do not add OpenCV to CMake unless compilation proves a source file requires it.

## Revision Notes

2026-05-03 / Codex: Initial plan created in response to the request for an implementation plan to move from qmake to CMake. The plan keeps qmake as a fallback, preserves the module structure, and separates build-system migration from Qt 6 API migration.

2026-05-03 / Codex: Executed the plan by adding CMake files for all modules, applying minimal Qt 6 compile fixes, validating configure/build success, adding generated outputs to `.gitignore`, and updating `AGENTS.md` to show CMake as the primary build path.
