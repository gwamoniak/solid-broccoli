---
name: sibling-projects
description: The two sibling projects in this sandbox (../Coffee_Dispenser and ../../llm/dobromir) — what they are, which conventions they share with solid-broccoli, and which laws differ. Use before reading, referencing, or making any change in a sibling project directory.
---

# Sibling projects (same sandbox, same maintainer)

Each sibling has its own `CLAUDE.md`. Read it before doing anything there — the rules below are orientation, not a substitute.

## `../Coffee_Dispenser`

Qt6/QML coffee-machine HMI, being modernized per its own `COFFEE_DISPENSER_MODERNIZATION_EXECPLAN.md` (authored 2026-07-05, M0 not started).

It deliberately reuses the patterns proven in solid-broccoli:

- CMake target layering
- sans-IO codecs
- simulator-first with fault injection
- thin views
- design tokens
- gated hardware milestones

It has its own `CLAUDE.md` and a `dispenser-builder` agent. The ExecPlan discipline applies there even though `PLANS.md` lives only in the solid-broccoli repo.

## `../../llm/dobromir`

Pre-alpha LLM/VLM project: photo of a problem → wordless IKEA-style repair schematic.

**Not Qt, and different laws** — do not carry solid-broccoli conventions across:

- `snake_case` naming
- no-training-before-eval
- learning milestones the maintainer implements *by hand*

See its `CLAUDE.md` and `ROADMAP.md` before doing anything there.
