# solid-broccoli — User Manual

A field guide to operating the instrument. For how the software is built, see
[ARCHITECTURE.md](ARCHITECTURE.md).

Everything in this manual works with **zero hardware and zero downloaded
models** — the app ships with high-fidelity simulated instruments precisely so
you can learn it (and develop it) on a bare machine. Sections marked
*(optional)* need something extra: a BLE bridge, an ONNX model, or a GGUF
model.

---

## 1. First launch

Build and run (from the repository root):

```bash
cmake -S QML_ML_Camera -B QML_ML_Camera/build
cmake --build QML_ML_Camera/build -j
./QML_ML_Camera/build/app/QML_ML_Camera_App.app/Contents/MacOS/QML_ML_Camera_App
```

The app opens on the **Live** tab: a dark instrument surface with a yellow
accent. Six tabs along the bottom:

| Tab | What it is |
|---|---|
| **Live** | Spectral viewfinder: live trace, readouts, analysis panel, RADIATION card |
| **Sessions** | The lab notebook: saved captures, measurements, videos, AI reports, exports |
| **Camera** | Photo/video capture with optional spectrum overlay and object detection |
| **Photos** | Photo albums |
| **Videos** | Video albums (including spectrum-overlaid recordings) |
| **Settings** | Devices, calibration-adjacent parameters, feature toggles, models |

Permission prompts you may see, each at the moment you first need it:
**Camera** when opening the Camera tab (macOS remembers a denial — see
[Troubleshooting](#8-troubleshooting)), **Bluetooth** the first time you tap
*Refresh* under Settings → SPECTROMETER.

Color language: **yellow** = selected/active/primary, **red** = recording,
radiation above your alert threshold, or a destructive action. Nothing else is
ever red.

## 2. Your first spectrum (60 seconds)

1. Live tab → press **Start**. The service auto-connects the default device —
   *Simulated UV-Vis (mercury lamp)* — and a moving yellow trace appears.
2. Read the strip above the plot: **PEAK λ** should hover at **546.1 nm**
   (the mercury green line), with PEAK / INTEGRATION / FPS beside it.
3. Hover or tap the trace to inspect the nearest measured wavelength and
   value; known local peaks also show their deterministic line candidate.
   Zoom with pinch or mouse wheel, pan by dragging, and use **Hold** to
   freeze/unfreeze without stopping acquisition.
4. Open the analysis panel (docked on the right at tablet width; the **⋯**
   button opens it as a drawer on narrow windows): the PEAKS list names every
   detected line — tap one to center the plot on it.
5. Press **Save** (in the panel), give the capture a name and optional tags —
   it lands in the current session, visible on the **Sessions** tab.

## 3. Absorbance measurements (dark / reference / sample)

Spectroscopy beyond "look at the trace" needs calibration captures. The
workflow mirrors a real bench instrument:

1. Settings → SPECTROMETER → tap the device row until *Simulated dye sample
   (Beer-Lambert)* is selected, then **Connect**.
2. Live tab → **Start**.
3. Press **Dark** — the simulator switches its lamp off and the noise floor is
   stored (with hardware: block the light path first).
4. Press **Ref** — captured through the blank (the simulator swaps the
   cuvette; with hardware: insert the blank).
5. Select **ABS** in the RAW / TRANS / ABS segment control. It refuses
   politely until both calibration captures exist.
6. You now see a real absorption band at **664 nm** (methylene-blue-like).
   In the analysis panel, set the integration region ("Set from view" uses
   the current zoom) and watch the live **∫** readout — its stability is your
   signal-quality indicator.
7. **Save** stores the capture *with* its dark and reference, once per
   session, so absorbance is reproducible after restart.

Integration time and averaging (Settings → SPECTROMETER) behave physically:
longer integration = more signal, more averaging = less noise. The smoothing
stepper in the analysis panel applies a Savitzky-Golay filter to the display.

## 4. Radiation monitoring

The Geiger counter is the second sensor modality; it shares sessions with the
spectrometer so dose measurements and spectra land in the same notebook entry.

- **Connect**: Settings → GEIGER → cycle to *Simulated Geiger (check source)*
  → Connect. The **RADIATION** card on the Live tab starts wobbling — that
  wobble is honest Poisson statistics, not decoration.
- **Read**: dose rate in **µSv/h** (large, monospaced; turns red above your
  alert threshold), CPM below it, a compact sparkline beside.
- **Tune** (Settings → GEIGER): *Tube factor* converts CPM → µSv/h
  (default 0.0057 for an SBM-20 tube; set yours from the tube datasheet);
  *Alert threshold* sets the red line (default 0.5 µSv/h).
- **Record**: tap the RADIATION card → the Geiger page shows the full strip
  chart with the threshold line. **Save** writes a windowed summary
  (average CPM, max CPS, dose, tube factor) into the session; **Export**
  writes a CSV (`timestamp_ms,cps,cpm,usv_per_h`).

The dose readout also appears in recorded video when the spectrum overlay is
on — a radiation-only chip when no spectrometer is running.

## 5. Camera, photos, video

- **Destination pill**: the album name on the Camera page is where photos
  will be filed. Tap it to pick another album, **＋** creates one. If you have
  no albums at all, the first capture auto-creates **Camera Roll** — captures
  are never silently dropped.
- **Photo**: the round shutter. **Video**: the record toggle — red while
  recording. Recordings made while a session is active are linked to it and
  listed on the session's detail page.
- **Spectrum overlay** (Settings → GENERAL): burns the live trace, peak λ,
  and dose into recorded video — the documentation feature: one clip shows
  what you saw *and* what the instrument read.
- Live-preview filters (e.g. Grayscale) are plugin toggles in Settings.

## 6. Object detection *(optional — needs ONNX Runtime + a model)*

Requires the app to have been built with ONNX Runtime present
(`brew install onnxruntime`, then reconfigure; Settings → VISION says
"not available" otherwise).

1. **Get a model** (YOLO v8/v11 ONNX). Either export locally:

   ```bash
   brew install python@3.12
   /opt/homebrew/opt/python@3.12/bin/python3.12 -m venv ~/yolo-venv
   ~/yolo-venv/bin/pip install ultralytics
   ~/yolo-venv/bin/yolo export model=yolo11n.pt format=onnx
   ```

   or download a pre-exported `yolo11n.onnx`.
2. Settings → VISION → **Import…** — the file is copied into the app's model
   store with its SHA-256 recorded.
3. Toggle **Object detection** on, open the Camera tab: yellow boxes + label
   chips appear on the preview.
4. *Detection stride* (1–10) trades freshness for battery: inference runs
   every Nth frame; boxes are drawn on every frame regardless, so preview
   never stutters.

Detections are also fed into AI reports (below) as "what the camera saw".

## 7. AI session reports *(optional — needs an AI build + a GGUF model)*

The report generator is **grounded**: peak identification comes from a
deterministic matcher against a curated reference-line library (mercury,
neon, sodium, hydrogen Balmer, argon, common laser diodes, methylene blue,
chlorophylls, water). The language model only turns those verified facts into
prose — it is never allowed to add or alter an identification, and every
report stores the exact JSON facts it was given, so you can audit any claim.

1. **Get a model**: recommended **Gemma 3 4B instruct QAT Q4_0** (~2.4 GB
   file, ~3 GB RAM; from `ggml-org/gemma-3-4b-it-qat-GGUF` on Hugging Face).
   On 8 GB machines use **Gemma 3 1B** (~0.7 GB). Note Gemma's license terms
   accompany the download.
2. Settings → AI → **Choose…** and pick the `.gguf`. The file is referenced
   in place — multi-gigabyte models are never copied.
3. Sessions tab → open a session that has captures → **Generate AI report**.
   First generation loads the model (seconds on Apple Silicon), then tokens
   stream live into the viewer.
4. The saved report appears under REPORTS with an **AI** badge — tap to read,
   **Export .md** writes it next to your CSV/JSON exports.

Every report ends with a fixed caveat: *AI-generated interpretation — verify
against calibration and reference standards.* That footer is forced by the
versioned prompt, not model goodwill.

Reports remain readable and exportable even in builds without the AI runtime —
only *generating* new ones needs it.

## 8. Troubleshooting

**Camera shows "Camera not active".** macOS denied camera access. System
Settings → Privacy & Security → Camera → enable the app. If it never appears
there (a previous auto-denial), reset the record and relaunch:
`tccutil reset Camera com.solidbroccoli.qmlmlcamera`.

**App crashes at launch with `dyld: Symbol not found … QtPrivate_6_11_0`.**
A partial Homebrew upgrade left Qt modules at mixed versions. Fix:
`brew upgrade qt` (all Qt formulae to the same version). To prevent it:
`brew pin qt qtbase qtdeclarative qtmultimedia qtconnectivity qtsvg`.

**My photos "disappear".** Photos are filed to the album shown in the Camera
page's destination pill — check that pill, not the album you happen to have
open on the Photos tab.

**BLE bridge doesn't appear.** Settings → SPECTROMETER → **Refresh** runs a
5-second scan; the device must advertise the bridge service UUID
(`5B0C0001-8E2B-4D8B-9C60-0A5B3D1EAD01`). First scan triggers the macOS
Bluetooth permission prompt — accept it. Meanwhile, the *Simulated AS7265x
bridge (loopback)* device exercises the identical byte path with no radio.

**ABS/TRANS is greyed out.** Capture **Dark** and **Ref** first (section 3) —
absorbance without calibration would be a lie, so the app refuses.

**Object detection toggle says "not available".** The app was built without
ONNX Runtime. `brew install onnxruntime`, delete nothing, reconfigure + rebuild;
the plugin target appears automatically.

**Report generation is slow.** Use the 1B model, or accept ~seconds of model
load on first generation only — the model stays resident afterwards. Changing
the GGUF in Settings unloads the old one.

**Where is my data?** Everything lives under the platform app-data root — on
macOS `~/Library/Application Support/SolidBroccoli/SolidBroccoli/`:

| Content | Location |
|---|---|
| Database (albums, sessions, everything) | `solidBroccoli_Gallery.db` |
| Logs (CSV, one per day) | `logs/SolidBroccoli_Log_<date>.csv` |
| Photos / recordings / thumbnails | `pictures/`, `recordings/`, `thumbnails/` |
| CSV / JSON / report exports | `exports/` |
| Imported ONNX models | `models/` |

Deleting the database file resets all app content (photos/videos on disk are
not deleted, but their catalog is). GGUF models are wherever you stored them —
the app only keeps a path.

**Can I read this manual offline?** Yes. Settings → **User Manual** opens this
same file from the app bundle; no browser or network connection is required.

**Something else is wrong.** Read today's CSV log (path above, or Settings →
the Logger page) — every service logs its decisions, and most support
questions are answered by the last ten lines.
