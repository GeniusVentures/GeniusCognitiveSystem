## Conflict Detection Report

### BLOCKERS (0)

No blockers found. All 29 documents are DOC type (high confidence, unlocked). The existing CONTEXT.md files belong to the `doc-template` workstream (a documentation template tool) and are unrelated to the GCS architecture docs being ingested.

### WARNINGS (0)

No competing acceptance variants. No PRD-type documents exist in this ingest set. No requirement overlap detected.

### INFO (3)

[INFO] Cross-reference cycles detected (DOC type -- informational only)

Cycle 1: `sgfp4-format.md` <-> `model-and-router.md` (2-node mutual reference)
  Source: docs/architecture/sgfp4-format.md cross_refs include ./model-and-router.md
  Source: docs/architecture/model-and-router.md cross_refs include ./sgfp4-format.md
  Impact: None. Both docs describe complementary technical topics (quantization format and model architecture). The bidirectional reference is intentional documentation structure, not a decision dependency cycle.

Cycle 2: `sgfp4-format.md` <-> `system-overview.md` (2-node mutual reference)
  Source: docs/architecture/sgfp4-format.md cross_refs include ./system-overview.md
  Source: docs/architecture/system-overview.md cross_refs include ./sgfp4-format.md
  Impact: None. System overview references the quantization format; the format spec references the system overview for architectural context.

Cycle 3: `frozen-mtp-and-vtg.md` <-> `speculative-decoding-and-vtg.md` (2-node mutual reference)
  Source: docs/architecture/frozen-mtp-and-vtg.md cross_refs include ./speculative-decoding-and-vtg.md
  Source: docs/architecture/speculative-decoding-and-vtg.md cross_refs include ./frozen-mtp-and-vtg.md
  Impact: None. Both are companion documents that together define the VTG/speculative decoding architecture. Each explicitly declares the other as a companion document to be read together.

[INFO] Merge mode: existing workstream context is unrelated

  The existing .planning files (PROJECT.md, ROADMAP.md, REQUIREMENTS.md, STATE.md) belong to the `doc-template` workstream -- a reusable MkDocs documentation template. The GCS architecture docs describe a distributed cognitive system. No overlap, no conflicts.

  Source: .planning/workstreams/doc-template/PROJECT.md defines "gendoc-template" project
  Source: .planning/STATE.md tracks doc-template v0.1 milestone (80% complete)

[INFO] Existing cross-submodule-capabilities.md is consistent with this ingest

  The file .planning/intel/cross-submodule-capabilities.md was generated from a previous ingest of the same docs/architecture/ source files. It identifies capabilities (Capability System, OpenAI API Router, Speculative Decoding/VTG, Objective Memory/VTG, Local Cognitive Second Brain, EGGROLL) that span multiple submodules. This ingest confirms those findings without contradiction.

  Source: .planning/intel/cross-submodule-capabilities.md (generated 2026-07-18)
