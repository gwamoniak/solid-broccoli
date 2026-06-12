# solid-broccoli

A Qt6/QML camera application with photo and video album management. It captures
images and video from a USB/system camera, stores metadata in SQLite, and
displays a QML gallery UI. The live preview can be transformed by runtime
frame-processor plugins (a grayscale proof plugin ships in `plugins/grayscale/`;
the interface is designed so OpenCV or deep-learning plugins can be added later
without the application itself linking those libraries).

## Build

The project uses CMake. From the repository root:

    cmake -S QML_ML_Camera -B QML_ML_Camera/build
    cmake --build QML_ML_Camera/build

Run the app:

    ./QML_ML_Camera/build/app/QML_ML_Camera_App.app/Contents/MacOS/QML_ML_Camera_App

Run the unit-test suite (from the build directory):

    cd QML_ML_Camera/build
    ctest --output-on-failure

**Dependencies:** Qt 6.5+ (Core, Gui, Qml, Quick, Sql, Svg, Multimedia, Test).
There is no OpenCV dependency in the current codebase.

## Icons

The toolbar/button icons in `QML_ML_Camera/app/images/svg/` are original,
single-color SVG glyphs authored for this project and released under
[CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/) (public domain).
Each is a monochrome shape recolored at runtime through a control's
`icon.color`, so one source serves any theme and renders crisply at any DPI.
The previous raster PNG icons were removed; they remain in git history.
