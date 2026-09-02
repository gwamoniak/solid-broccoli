---
name: qmllint-baseline
description: The qmllint warning baseline for solid-broccoli after registered-singleton and layout cleanup. Use when running qmllint, triaging its output, or deciding whether an unqualified-scope warning is pre-existing or a regression.
---

# qmllint baseline

Run after any QML change:

```bash
/opt/homebrew/opt/qt/bin/qmllint -I QML_ML_Camera/qmltypes QML_ML_Camera/app/*.qml
```

**The rule: introduce no new warnings and drive the remaining unqualified-scope baseline down deliberately.**

The current baseline is 141 `[unqualified]` warnings. There are zero module
import, unresolved-type, missing-property, unused-import, confusing-expression,
or `Quick.layout-positioning` warnings.

## Remaining baseline — parent/delegate scope (141)

The C++ objects are registered under `solid.broccoli 1.0` and described to
tooling by `QML_ML_Camera/qmltypes/solid/broccoli/plugins.qmltypes`. The
remaining warnings are older implicit parent-property and delegate-role
lookups. They work at runtime but can be made explicit with component ids or
`required property` declarations when those pages are next edited.

Do not suppress the category globally: a new unqualified service/model name is
usually a real wiring error. Compare the exact count and inspect any new line.

## Resolved baseline

The former 29 `Quick.layout-positioning` warnings were corrected with
`Layout.preferredWidth` / `Layout.preferredHeight`. Context properties were
replaced by `AppContext` and registered service singletons. Missing C++ type
metadata is no longer an accepted explanation for import or unresolved-type
warnings.

## Real findings — fix now

Everything else is a genuine defect to fix in the change that introduced it:

- `[unused-imports]`
- `[missing-property]` — e.g. a real `Qt.AlignTop` typo found this way
- `[confusing-expression-statement]` — e.g. a block binding missing its `return`
- duplicate or invalid signal overrides
