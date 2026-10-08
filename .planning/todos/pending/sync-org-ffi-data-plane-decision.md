---
title: Sync org-wide FFI data plane decision into genius-tube and genius-ai-boss planning
date: 2026-08-26
priority: medium
context: gsd-explore (GCS D-29)
resolves_phase: 0
---

# Sync org-wide FFI data plane decision into sibling repos

GCS's D-29 (`.planning/workstreams/app/phases/01-foundation/01-CONTEXT.md`, 2026-08-26) cites an
org-wide FFI data plane decision as "frozen by genius-tube D-27/D-28, adopted by genius-ai-boss
D-20/D-20a". Verified 2026-08-26: **those decisions are not written in either sibling repo's
planning tree** (searched both `.planning` dirs — genius-tube's 00-CONTEXT has D-21 but no
D-27/D-28/codec text; genius-ai-boss has no D-20a). GCS D-29 is currently the only canonical
statement.

## Org-wide pattern (as locked in GCS D-29)

- In-process topic pub/sub across the C++↔Dart FFI boundary; minimal C ABI: init / publish / subscribe.
- C++ core owns application state; Dart Cubits are thin topic subscribers; commands are topic publishes.
- Wire payloads are codec-tagged bytes (`uint8_t* + length`, never `char*`); codec bound per store at creation.
- **Per-store codec choice — JSON is not exclusive, it is used when the data is JSON-native** (user 2026-08-26). genius-ai-boss binds JSON on its OpenAPI-spec'd backend channels (can bind protobuf per channel when needed); GCS stores bind protobuf (chat + future wallet widget hold GeniusSDK proto types natively); genius-tube uses protobuf per its D-03a/D-25.
- Schema-source canonicality is a per-project choice (GCS: authored `.proto`; genius-tube: OpenAPI/Jinja2 sources). If an OpenAPI artifact is wanted alongside protobuf, generate it proto→OpenAPI (standard tooling direction) — never author OpenAPI as the proto source.

## To do

- [ ] Write the codec-tagged-bytes refinement into genius-tube's planning as their D-27/D-28 (or whatever numbering their next context revision uses).
- [ ] Write the adoption into genius-ai-boss's planning as their D-20/D-20a.
- [ ] Update GCS D-29's parenthetical + this todo once sibling refs resolve.
