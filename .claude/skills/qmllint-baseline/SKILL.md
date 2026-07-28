---
name: qmllint-baseline
description: The qmllint warning baseline for solid-broccoli — what the ~387 current warnings are, which of the three buckets each belongs to, and which are real findings to fix now. Use when running qmllint, triaging its output, or deciding whether a warning is pre-existing noise or a genuine regression.
---

# qmllint baseline

Run after any QML change:

```bash
/opt/homebrew/opt/qt/bin/qmllint QML_ML_Camera/app/*.qml
```

**The rule: introduce no new bucket-2 or bucket-3 warnings, and drive the existing bucket-2 count down deliberately.**

The current baseline is ~387 warnings in three buckets.

## Bucket 1 — structural, not actionable today (~358)

Context properties (`albumModel`, `reportService`, `owningStack`, …) and the C++-registered `SpectrumView` / `StripChartView` types are invisible to qmllint, so every use is flagged `[unqualified]`, `[import]`, or `[unresolved-type]`.

The recorded fix is exposing the app as a QML module and migrating context properties to registered singletons — that's a hardening-backlog item, not something to attempt piecemeal. Ignore these.

## Bucket 2 — real, pre-existing (~29 `[Quick.layout-positioning]`)

Setting `width` / `height` on a Layout-managed item. Qt calls this undefined behavior — use `Layout.preferredWidth` / `implicitWidth` instead.

These span `AnalysisPanel`, `CameraPage`, `GeigerPage`, `LivePage`, and `SessionDetailPage`. They want a *verified* cleanup pass, not a blind sweep: fixing them can shift layouts, so each change needs to be looked at running. Recorded as backlog.

## Bucket 3 — real findings, fix now

Everything else is a genuine defect to fix in the change that introduced it:

- `[unused-imports]`
- `[missing-property]` — e.g. a real `Qt.AlignTop` typo found this way
- `[confusing-expression-statement]` — e.g. a block binding missing its `return`
- duplicate or invalid signal overrides
