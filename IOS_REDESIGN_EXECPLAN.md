# Redesign the desktop app UI into a modern iOS look: tab-bar shell, light theme, teal accent

This ExecPlan is a living document. The sections `Progress`, `Surprises & Discoveries`, `Decision Log`, and `Outcomes & Retrospective` must be kept up to date as work proceeds. It follows the repository guidance in `PLANS.md` (repository root) and must be maintained in accordance with it.

## Purpose / Big Picture

Today the application (built from `QML_ML_Camera/` with CMake, producing `QML_ML_Camera_App`) works, but its UI is a six-year-old prototype: a purple window with tan tool bars top and bottom, brightly colored circular buttons floating along the right edge, a back button in the top-right corner, and a home button centered at the bottom. Navigation is a hub screen (`MainPage`) of floating buttons that push other pages onto a `StackView`.

After this plan, the same features are presented as a modern, iOS-style application that a current macOS/iOS user finds instantly familiar:

- The window opens on a **bottom tab bar** with four tabs — Photos, Videos, Camera, Settings — instead of a hub screen of floating circles.
- Each tab has its own **navigation bar** with a large title, a leading back chevron on the left (iOS convention), and trailing actions on the right (for example, a "+" to add an album).
- The palette is **iOS light mode**: a system grouped background (`#F2F2F7`), white surfaces, black labels, hairline separators, with a single **teal accent** (`#1D9E75`) and **system red** (`#FF3B30`) for destructive actions.
- Albums appear as a clean rounded-card grid; photos as an edge-to-edge square grid like the iOS Photos app; the camera as a full-bleed viewfinder with a round shutter and a Photo/Video mode switch; settings as grouped inset table sections.
- Navigation animates with iOS-style push/pop slide transitions.

This is a **view-layer-only** redesign. No C++ logic, models, DAOs, `CameraService`, `AppSettings`, image provider, or database code changes behavior. The thin-view rule the project already follows (all logic in C++; QML is layout, styling, bindings, and invokable calls) is preserved and reinforced.

How to see it working at the end: build and run the app; it opens on the Photos tab with a large "Photos" title and a bottom tab bar; tapping Camera shows a full-bleed viewfinder; tapping Settings shows grouped rows with the existing functional toggles; deleting an album uses an iOS-style confirmation; moving between screens slides like iOS.

## Progress

- [x] (2026-06-12) Planning: requirements gathered, design direction chosen (bottom tab bar, light mode, teal/green accent), mockup produced and shown, this ExecPlan authored.
- [ ] Milestone 1: Design-system foundation — `Theme` tokens (color, type scale, metrics) with back-compat shims so existing pages still build.
- [ ] Milestone 2: App shell — bottom `TabBar` + per-tab navigation stacks and a reusable `NavPage` (iOS nav bar), replacing `MainPage`, `PageTheme`, and `ToolBarTheme`.
- [ ] Milestone 3: Photos & Videos tabs — album grid, photo grid, picture detail, recordings list + player, iOS edit/delete and "+" add.
- [ ] Milestone 4: Camera tab — full-bleed viewfinder, bottom control bar (mode switch, round shutter, record toggle, settings affordance).
- [ ] Milestone 5: Settings tab + dialogs — grouped inset table bound to `AppSettings`; iOS alert/sheet replacing `InputDialog`.
- [ ] Milestone 6: Motion, icons, and polish — push/pop transitions, SF-symbol-like icon set, empty/loading states, final pass.

## Surprises & Discoveries

- Observation: The whole UI is already a thin view over C++ singletons and models (`albumModel`, `pictureModel`, `movieModel`, `loggerModel`, `CameraService`, `AppSettings`, `image://pictures`), so the redesign touches QML only. The data contracts the new UI must bind to are stable and enumerated under `Interfaces and Dependencies`.
  Evidence: `main.cpp` registers exactly those context properties/singletons; pages call invokables like `albumModel.addAlbumFromName`, `pictureModel.setAlbumId`, `CameraService.captureImage`, `AppSettings.shutterFlash`.
- Observation: `StackView` transitions are currently disabled (`pushEnter: null`, etc. in `main.qml`), which is why navigation feels instant/static. Re-enabling them is a Milestone 6 task, not a bug.
  Evidence: `QML_ML_Camera/app/main.qml` lines 30–35.
- (To be filled during implementation.)

## Decision Log

- Decision: Adopt a bottom `TabBar` (Photos / Videos / Camera / Settings) as the primary navigation, replacing the `MainPage` hub and the floating circular buttons.
  Rationale: User choice (2026-06-12). It is the canonical iOS pattern and removes the most dated element of the current UI.
  Date/Author: 2026-06-12 / Claude + maintainer
- Decision: Default appearance is iOS **light** mode; dark mode is out of scope for this plan (the token layer is structured so a dark palette can be added later without touching pages).
  Rationale: User choice (2026-06-12). Keeps scope bounded while leaving the door open.
  Date/Author: 2026-06-12 / Claude + maintainer
- Decision: Accent color is **teal** `#1D9E75` (pressed `#0F6E56`); destructive actions use system red `#FF3B30`.
  Rationale: User choice of a camera-app-friendly teal/green. `#1D9E75` matches the mockup and is reused verbatim as the single accent token.
  Date/Author: 2026-06-12 / Claude + maintainer
- Decision: Build the iOS look from **custom QML components** styled by a `Theme` singleton, rather than relying on a platform Qt Quick Controls style.
  Rationale: Qt's native `iOS` Controls style is only available on iOS; on desktop the available styles (Basic/Fusion/macOS) do not give the exact look and differ per platform. Custom components keyed off one token file give a consistent, controllable result and match how the project already styles controls inline.
  Date/Author: 2026-06-12 / Claude
- Decision: Introduce the design tokens first (Milestone 1) with temporary back-compatibility aliases for the existing `Style.qml` property names, then migrate pages tab-by-tab.
  Rationale: Keeps the app building and runnable at every milestone boundary (a PLANS.md requirement) instead of a single big-bang rewrite.
  Date/Author: 2026-06-12 / Claude
- Decision: Keep the macOS desktop window resizable; lay out with anchors/Layouts so the tab bar stays pinned to the bottom and content reflows, rather than hard-coding the current 1280x800.
  Rationale: A modern app should not assume a fixed canvas; iOS layouts are intrinsically adaptive.
  Date/Author: 2026-06-12 / Claude

## Outcomes & Retrospective

- (2026-06-12) Planning complete. Design direction agreed and visualized; this plan decomposes the redesign into six independently verifiable, always-building milestones that touch only the QML view layer. Implementation not yet started.

## Context and Orientation

The repository root contains `QML_ML_Camera/`, which builds with CMake (`QML_ML_Camera/CMakeLists.txt`) into `logger-core`, `camera-core`, `processor-api`, the `grayscale-processor` plugin, and the `QML_ML_Camera_App` executable. Only the executable's QML and a couple of registration lines in `QML_ML_Camera/app/main.cpp` are in scope here.

Key files in `QML_ML_Camera/app/` as they exist today (full repository-relative paths):

- `main.qml` — `ApplicationWindow` (1280x800, `color: Style.windowBackground`) holding a single `StackView` with `initialItem: MainPage{}` and all transitions disabled. Exposes `pageStack` and `goHome()`.
- `MainPage.qml` — the hub: a settings gear (top-left), a vertical column of colored `RoundButton`s (Albums, Videos, Camera, Quit) on the right, and a right-edge `Drawer` (recently modernized) containing a camera `ComboBox`, two `SettingSwitcher` toggles bound to `AppSettings`, and an "Open Logger" button.
- `PageTheme.qml` — base `Page` providing a custom 86px `header` (an 86px tan `ToolBarTheme`) with a centered title, a leading-area-less layout, a back `RoundButton` in the top-right, and a `Loader` (`toolbarButtons`) for page-specific buttons stacked vertically on the right; plus a 74px tan `footer` with a centered home `RoundButton`.
- `ToolBarTheme.qml` — a `ToolBar` whose background is `Style.toolBackground` (tan).
- `Style.qml` — `pragma Singleton` of colors and metrics: `text`, `pictureText`, `windowBackground` (`#bd93f9` purple), `toolBackground` (`#C0BC87` tan), `pageBackground` (`#6272a4` blue-grey), `buttonBackground` (`#1e90ff`), `iconColor`, `dangerColor`, `iconSize`, `fontName`, `fontSize`, and `roundButton*` sizes/colors. Registered as a singleton via `qmldir`.
- `AlbumListPage.qml` — `PageTheme` with a `ListView` over `albumModel`; each row is a flat colored `Rectangle` with the album name; an `InputDialog` adds an album.
- `AlbumPage.qml` — `PageTheme` with a `GridView` over `pictureModel` showing `image://pictures/<index>/thumbnail`; toolbar buttons add/rename/delete; a `FileDialog` imports a picture.
- `PicturePage.qml` — single picture view with rename/delete.
- `MovieAlbumPage.qml` / `MoviePage.qml` — recordings list and a Qt6 `MediaPlayer` + `AudioOutput` player with seek/volume/rename/delete.
- `CameraPage.qml` — `PageTheme` with a `VideoOutput` (bound to `CameraService` via `attachVideoOutput`), a settings `Drawer` (device/format combos + a "Processors" plugin list), a shutter flash overlay (gated by `AppSettings.shutterFlash`), a mirror transform (`AppSettings.mirrorPreview`), and round capture/record/settings buttons.
- `LoggerPage.qml` — shows log entries from `loggerModel`; opens the logs folder (`logsPath`).
- `SettingSwitcher.qml` — a modern row: label + iOS-style pill toggle, with `value`/`toggled` contract.
- `InputDialog.qml` — a dark modal `Dialog` with a `TextField` and Cancel/OK.
- `images/svg/` — the current monochrome SVG icon set (add-album, add-photo, back, camera, delete, gallery, home, log, movie, quit, record, rename, save, settings), recolored via `icon.color`.

"Singleton" here means a QML type registered once and shared (via `qmldir` for `Style`, via `qmlRegisterSingletonInstance` for `CameraService`/`AppSettings`). "Token" means a single named design value (a color, size, or font) defined once in one file and referenced everywhere, so the whole look changes from one place. "Nav bar" is the top bar of a screen (title + back + actions); "tab bar" is the bottom bar that switches top-level sections.

Build and run (working directory: repository root):

    cmake -S QML_ML_Camera -B QML_ML_Camera/build
    cmake --build QML_ML_Camera/build -j
    ./QML_ML_Camera/build/app/QML_ML_Camera_App.app/Contents/MacOS/QML_ML_Camera_App

## Plan of Work

The work proceeds as six milestones. Each ends with the app building and running, and each is independently verifiable. The guiding principle: introduce the new system additively, migrate screen-by-screen, and delete the old chrome only once nothing references it.

### Milestone 1 — Design-system foundation

Goal: one place defines the entire iOS look, and the app still builds with the old pages untouched.

Create `QML_ML_Camera/app/Theme.qml` (a new `pragma Singleton`, registered in `qmldir` alongside `Style`) holding the iOS token set in plain, named properties. Define exactly these values so the result is reproducible:

- Colors (light mode): `accent: "#1D9E75"`, `accentPressed: "#0F6E56"`, `destructive: "#FF3B30"`, `groupedBackground: "#F2F2F7"` (screen background), `surface: "#FFFFFF"` (cards/rows/nav/tab bars), `label: "#000000"`, `secondaryLabel: "#8E8E93"`, `tertiaryLabel: "#C7C7CC"`, `separator: "#C6C6C8"`, `fill: "#E3E3E8"` (search/segmented backgrounds), `onAccent: "#FFFFFF"`.
- Type scale (point sizes): `largeTitle: 30`, `title: 22`, `headline: 17`, `body: 17`, `callout: 16`, `subhead: 15`, `footnote: 13`, `caption: 11`. Font family `fontName: ""` (empty string → the platform system font, which is San Francisco on macOS).
- Metrics: `navBarHeight: 52`, `tabBarHeight: 56`, `rowHeight: 44`, `screenMargin: 16`, `gridGap: 12`, `radiusCard: 12`, `radiusControl: 10`, `radiusSheet: 14`, `radiusThumb: 10`, `hairline: 1`, `shutterSize: 66`, `accentRing: 4`.

In the same milestone, repoint `Style.qml`'s existing properties at the new tokens (for example, make `Style.pageBackground` return `Theme.groupedBackground`, `Style.text` return `Theme.label`, `Style.buttonBackground` return `Theme.accent`, `Style.dangerColor` return `Theme.destructive`) so the legacy pages immediately render in the new palette without being rewritten yet, and nothing breaks. Keep `Style` as the compatibility shim; new components reference `Theme` directly.

Acceptance: `cmake --build` succeeds; launching the app shows the existing layout but recolored toward the light/teal palette (proof the token layer is wired). No page rewrites yet.

### Milestone 2 — App shell: tab bar + per-tab navigation

Goal: replace the hub-and-floating-buttons model with a bottom tab bar and iOS nav bars.

Rewrite `QML_ML_Camera/app/main.qml` so the `ApplicationWindow` contains a footer `TabBar` (height `Theme.tabBarHeight`, `surface` background, hairline top separator) with four `TabButton`s — Photos, Videos, Camera, Settings — each an icon above an 11px label, tinted `Theme.accent` when current and `Theme.secondaryLabel` otherwise. Above it, a `StackLayout` whose `currentIndex` follows the tab bar holds four independent `StackView`s, one per tab, so each tab keeps its own back history (the iOS model). Keep `goHome()` as "pop this tab's stack to its root" for compatibility with any remaining callers.

Create `QML_ML_Camera/app/NavPage.qml`, a reusable replacement for `PageTheme`: a `Page` whose `header` is an iOS nav bar — height `Theme.navBarHeight`, `surface` background with a bottom hairline, a leading back chevron (`ti`-style chevron icon, `Theme.accent`) shown only when the owning `StackView.depth > 1`, a centered title (`headline`), an optional large-title row below it (`largeTitle`, leading-aligned) toggled by a `largeTitle: bool` property, and a trailing slot (`property Component trailing`) for page actions like "+". No footer (the tab bar is global now). Expose `title`, `largeTitle`, `trailing`, and a `contentItem`/default property for page body.

Replace `MainPage.qml`'s role: the Photos tab's root becomes the album list (Milestone 3 restyles it); for this milestone it may temporarily host the existing `AlbumListPage` content adapted to `NavPage` so the shell is demonstrable. Remove the floating `RoundButton` column and the right-edge drawer from the navigation model (the drawer's contents move to the Settings tab in Milestone 5; keep the file until then).

Acceptance: the app opens on the Photos tab with a large "Photos" title and a bottom tab bar; tapping Videos/Camera/Settings switches tabs (placeholder content is fine for tabs not yet built); pushing a page within a tab shows a working leading back chevron; the tan tool bars, the home button, and the floating circular buttons are gone from view.

### Milestone 3 — Photos & Videos tabs

Goal: the gallery browsing experience looks and behaves like iOS Photos.

Photos tab. Restyle the album list as a 2-column rounded-card grid (`GridView` over `albumModel`): each card is a `surface` rounded rectangle (`radiusCard`) showing a cover thumbnail (the album's first picture via `image://pictures`, or a neutral placeholder), the album name (`subhead`, `label`), and the photo count (`caption`, `secondaryLabel`). The nav bar's trailing slot holds a "+" that opens the new-album dialog (the iOS dialog arrives in Milestone 5; until then reuse `InputDialog`). Tapping a card pushes the album's photo grid: an edge-to-edge square `GridView` over `pictureModel` (cells sized to the window width, `gridGap` spacing), tapping a photo pushes the single-picture view. Add an "Edit" affordance in the nav bar that reveals delete controls (a red badge per cell, or per-row swipe in list contexts), calling `pictureModel.removeRows`/`albumModel.removeRows`. Rename uses the Milestone 5 dialog.

Videos tab. Restyle the recordings list (`MovieAlbumPage` content) as an inset list of rows (thumbnail/placeholder + name + duration), pushing the existing `MoviePage` player, itself restyled to `NavPage` + `Theme` (keep the working `MediaPlayer`/`AudioOutput` wiring and seek/volume; restyle controls to teal). Delete/rename via the same iOS patterns.

Acceptance: browse albums as cards; open an album and scroll a square photo grid without stalls (the async `image://pictures` provider already handles this); open a single photo; enter Edit and delete a photo/album with a red confirmation; the Videos tab lists recordings and plays one back; visuals match the agreed mockup.

### Milestone 4 — Camera tab

Goal: a full-bleed iOS-style camera.

Rebuild `CameraPage.qml` as the Camera tab root using `NavPage` with the nav bar hidden or minimized (camera is chromeless). The `VideoOutput` fills the screen (keep `CameraService.attachVideoOutput`, the mirror transform bound to `AppSettings.mirrorPreview`, and the shutter flash overlay gated by `AppSettings.shutterFlash`). A bottom control bar over the viewfinder holds: a Photo/Video segmented mode switch, a large round shutter (`shutterSize`, white with an `accentRing` teal ring) that calls `CameraService.captureImage()` in Photo mode and toggles `startRecording()/stopRecording()` in Video mode (red while recording, with the live `recordingDuration`), a small thumbnail of the last capture (optional), and a settings affordance that opens the device/format/processors panel as a bottom sheet (reusing the existing combos and the "Processors" plugin `ListView`). Keep `Component.onCompleted`/`onDestruction` activation logic intact.

Acceptance: the Camera tab shows a full-bleed viewfinder; the shutter captures a photo (flash overlay respects the setting); switching to Video and pressing the shutter records and shows the timer; the settings sheet switches device/format and toggles the grayscale processor; nothing in `CameraService` changed.

### Milestone 5 — Settings tab + dialogs

Goal: a real iOS Settings screen and native-feeling dialogs.

Build the Settings tab as a grouped inset table (sectioned `Column`/`ListView` of rounded `surface` groups on the `groupedBackground`): a CAMERA section with a device row (the camera `ComboBox`, or a disclosure row pushing a picker), a GENERAL section with the two `SettingSwitcher` toggles bound to `AppSettings.shutterFlash`/`mirrorPreview`, and an ABOUT section with an "Open Logger" disclosure row pushing `LoggerPage` (restyled). This absorbs everything the old `MainPage` drawer held, after which the drawer and `MainPage.qml` can be deleted.

Replace `InputDialog.qml` with an iOS-style alert/sheet: a centered rounded `surface` card (`radiusSheet`) with a title, a single-line `TextField`, and Cancel / confirm actions (confirm in `accent`, destructive in `destructive`), over a dimmed scrim. Use it for new-album and rename across Photos/Videos. Restyle `LoggerPage` to `NavPage` + `Theme`.

Acceptance: the Settings tab shows grouped sections; toggling Shutter flash / Mirror preview persists (verified by relaunch) and affects the camera; "Open Logger" opens the restyled logger; creating and renaming albums/photos uses the new dialog.

### Milestone 6 — Motion, icons, and polish

Goal: the app feels alive and consistent.

Enable iOS-style navigation transitions on each tab's `StackView` (horizontal slide for push/pop) and a quick cross-fade when switching tabs. Refine the SVG icon set in `images/svg/` to an SF-Symbols-like thin, consistent line family (tab icons: photo, video, camera, settings; nav icons: chevron-left, plus, ellipsis; action icons: trash, pencil, share) and ensure every icon is tinted via `icon.color`/`Theme`. Add empty states (e.g., "No Albums" with a "+ Create Album" call to action; "No Recordings"), pressed/highlight states on rows and buttons, and a final spacing/typography pass against the mockup. Remove now-dead files (`PageTheme.qml`, `ToolBarTheme.qml`, `MainPage.qml`, old `Style` keys that no longer have consumers) and the `windowBackground`/tan/round-button tokens.

Acceptance: pushing/popping and tab switches animate smoothly; icons are visually consistent; empty and pressed states exist; `grep` finds no references to the removed files/tokens; the app matches the mockup across all four tabs.

## Concrete Steps

All commands run from the repository root. After every milestone:

    cmake -S QML_ML_Camera -B QML_ML_Camera/build
    cmake --build QML_ML_Camera/build -j
    ./QML_ML_Camera/build/app/QML_ML_Camera_App.app/Contents/MacOS/QML_ML_Camera_App

New QML files must be added to the `qt_add_resources` `FILES` list in `QML_ML_Camera/app/CMakeLists.txt` (and singletons like `Theme.qml` registered in `QML_ML_Camera/app/qmldir`), otherwise `qrc:/...` loads fail at runtime. Lint QML before running:

    /opt/homebrew/opt/qt/bin/qmllint QML_ML_Camera/app/*.qml

Because the app needs camera permission and a visible window to fully exercise, interactive screens (preview, capture, playback) are verified by launching the bundle and observing; non-visual wiring is verified via the CSV log under `~/Library/Application Support/SolidBroccoli/SolidBroccoli/logs/`.

## Validation and Acceptance

Overall acceptance is behavioral and cumulative: from a clean build, the app opens on a bottom-tab-bar shell (Photos/Videos/Camera/Settings), light theme with teal accent; albums browse as cards, photos as an edge-to-edge grid, a photo opens full-screen; the camera tab is a full-bleed viewfinder whose shutter captures (flash respects the setting) and whose Video mode records with a timer; the Settings tab shows grouped sections with the working `AppSettings` toggles and an Open Logger row; new-album/rename use an iOS dialog; navigation animates. Each milestone's own acceptance paragraph is the gate for checking its box in `Progress`. The existing CTest suite (`ctest --output-on-failure` from `QML_ML_Camera/build`) must remain green throughout, since this plan does not touch the tested C++.

## Idempotence and Recovery

Every step is an additive QML edit plus CMake resource-list additions, safe to re-run; CMake reconfiguration is idempotent. The token shim in Milestone 1 keeps old pages working, so the tree is runnable at each boundary and any milestone can be reverted independently on a feature branch. Deletions of dead files happen only in Milestone 6, after grep confirms no references. No C++, database, or on-disk data formats change, so there is no migration and no data risk.

## Artifacts and Notes

The agreed visual direction was shown as a three-screen mockup (Photos albums grid, Camera viewfinder, grouped Settings) in light mode with the teal accent and bottom tab bar; this plan's tokens (accent `#1D9E75`, grouped background `#F2F2F7`, etc.) match that mockup exactly.

## Interfaces and Dependencies

The redesign binds to these existing, unchanged C++ surfaces (do not modify them):

- Context properties (registered in `QML_ML_Camera/app/main.cpp`): `albumModel` (`addAlbumFromName`, `rename(row,name)`, `removeRows(row,count)`, roles `id`,`name`), `pictureModel` (`setAlbumId(id)`, `getAlbumId()`, `addPictureFromUrl(url)`, `rename`, `removeRows`, roles `name`,`filepath`,`url`), `movieModel` (`addRecording`, roles for url/name/duration), `loggerModel`, and `logsPath` (a URL).
- QML singletons: `CameraService` (`solid.broccoli 1.0`) with `availableCameras`/`currentCameraIndex`/`availableFormats`/`currentFormatIndex`/`active`/`recording`/`recordingDuration` and invokables `attachVideoOutput`, `setActive`, `captureImage`, `startRecording`, `stopRecording`, plus signals `imageSaved`/`captureError`/`recordingSaved`; `AppSettings` (`shutterFlash`, `mirrorPreview`, both read/write with NOTIFY); `pluginModel` (the `PluginManager` list model with `name`/`description`/`enabled` and `setEnabled(row,enabled)`).
- Image provider: `image://pictures/<row>/<thumbnail|full>` (async).
- New QML, added under `QML_ML_Camera/app/`: `Theme.qml` (singleton, in `qmldir`), `NavPage.qml`, an iOS `TabBar` in `main.qml`, restyled page files, and an iOS dialog component replacing `InputDialog.qml`. All new `.qml` files must be listed in `QML_ML_Camera/app/CMakeLists.txt` resources.
- Toolchain: Qt 6.5+ (local 6.11), Qt Quick + Controls + Multimedia, already required by the build. No new third-party dependency.
