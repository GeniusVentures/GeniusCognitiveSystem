---
name: solid-coupling
description: "SOLID/anti-coupling discipline for GCS: domain rules live in src/lib, never in src/ffi or src/app. Apply when planning, writing, or reviewing any C++ or Dart change."
---

# SOLID / anti-coupling discipline (GCS)

Binding on every GSD agent that plans, writes, or verifies code in this repo.
Full policy: CLAUDE.md "Hard invariants" (one owning module, never duplicate
domain knowledge, no domain logic in adapters, SOLID discipline #9). Evidence
for why this skill exists: `.planning/notes/whole-project-coupling-audit.md`
(2026-11-03 — the FFI layer had accumulated join derivation, role gates,
topic prefixes, and dormancy rules as parallel implementations of lib logic).

## The fixed layer map

| Layer | Directory | May do | May never do |
|---|---|---|---|
| Domain | `src/lib/**` | Own every rule, formula, threshold, prefix, state transition | Reach up into FFI/Dart concerns |
| Adapter | `src/ffi/**` | Translate handles/payloads, call lib, push events | Compute or re-decide a domain outcome |
| Presentation | `src/app/**` | Display values the adapter/lib computed | Re-implement a rule (predicate, regex, threshold) in Dart |

Coupling runs **upward only**: a rule may move from adapter into lib, never
from lib into adapter.

## Before writing a task's code (executor)

1. Name the rule the change introduces (join policy, role gate, name-length
   limit, dormancy window, key prefix…). If you cannot name it, it is plumbing.
2. `grep -rn` the rule's expression (literal, constant, threshold, format
   string) across `src/` — every existing home must be listed.
3. The named rule lands in its owning `src/lib` module **in the same change**;
   adapters and Dart call the result. If the plan put it in the adapter, the
   plan is wrong — raise a deviation rather than implementing it there.
4. Add/extend the test at the lib module's boundary (its own test target), not
   only in the FFI or widget tests.

## Before approving a plan (planner / plan-checker)

- Every task that adds behavior must say which `src/lib` module owns it. A
  plan whose new rule lives only in `gcs_core_ffi.cpp` or in Dart fails this
  check.
- A task that "just filters/derives in the adapter" is the audit's exact
  failure mode — reject or redirect it to the lib owner.

## The grep tripwire (executor and reviewer, pre-commit)

A changed `src/ffi/**` or `src/app/lib/**` file must not ADD:

- domain literals: `"gcs/"` topic or key prefixes, role names, limits,
  dormancy/timeout thresholds, invite-link patterns;
- rule arithmetic or boolean policy (`joined = …`, `roleAtLeast(…)`,
  `isDormant = …` re-implementations).

If it does, move the rule to `src/lib` and call it. Duplicate-but-agreeing
code is still a defect (the copies drift — see the audit's dormancy finding).
