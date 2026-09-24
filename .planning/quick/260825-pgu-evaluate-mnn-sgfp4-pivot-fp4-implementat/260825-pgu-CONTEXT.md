# Quick Task 260825-pgu: Evaluate MNN sgfp4-pivot FP4 implementation integration with GNUS-NEO-SWARM neo workspace FP4 codec and SGProcessingManager verification layer; assess MNN-side implementation quality; scope potential new GNUS-NEO-SWARM workstream for required updates - Context

**Gathered:** 2026-08-25
**Status:** Ready for planning

<domain>
## Task Boundary

Evaluate how MNN's `sgfp4-pivot` workstream (implementing the FP4 codec described in
`.planning/sgfp4-arxiv-v2.{pdf,txt}` at `W:\gnus\GeniusNetwork\thirdparty\MNN`) integrates
with GNUS-NEO-SWARM's own FP4 implementation (`GNUS-NEO-SWARM/src/core/fp4/fp4_codec.*`)
and the SGProcessingManager/SuperGenius verification layer that sits between
GeniusCognitiveSystem and MNN (`GNUS-NEO-SWARM/src/core/sgprocessing/*`). Assess how well
the MNN side of sgfp4-pivot was executed (architecture, phase completion, test coverage,
fidelity to the arxiv spec). Determine whether GNUS-NEO-SWARM's FP4 codec is compatible
with / already updated for MNN's current sgfp4-pivot output, and identify concrete gaps.

</domain>

<decisions>
## Implementation Decisions

### Deliverable scope
- Produce a written evaluation report only. Do NOT scaffold a new GSD workstream as part
  of this quick task.
- If the evaluation concludes a new workstream is warranted (to track GeniusCognitiveSystem
  / SuperGenius / SGProcessingManager updates needed to catch up with MNN's sgfp4-pivot),
  the report must say so explicitly and recommend it as a follow-up — but must not create
  the workstream files itself.

### Workstream location (if/when created later, as a follow-up — not in this task)
- Should live as a new top-level entry under `GeniusCognitiveSystem/.planning/workstreams/`
  (sibling to the existing `neo` / `neo-poc` entries), pointing at the GNUS-NEO-SWARM
  submodule the same way `.planning/workstreams/neo/config.json` does — not nested inside
  `GNUS-NEO-SWARM/.planning/workstreams/`.

### Claude's Discretion
- Report structure/format (sections, level of technical depth) is left to the planner/
  researcher — organize around: MNN-side quality assessment, GNUS-NEO-SWARM FP4 codec
  compatibility, SGProcessingManager verification-layer implications, and a clear
  yes/no recommendation on the new workstream with rationale.

</decisions>

<specifics>
## Specific Ideas

Known landscape from initial recon (for the researcher to verify/expand, not to take on faith):
- MNN: `.planning/workstreams/sgfp4-pivot/{REQUIREMENTS,ROADMAP,STATE}.md` with 4 phases
  (affine dual-mode decode core CPU uniform layouts; adaptive quadtree layout CPU mixed;
  Vulkan decode uniform layouts; Vulkan decode adaptive quadtree mixed layouts).
- MNN: `.planning/quick/260821-p1q-evaluate-current-fp4-ultra-fp4-implement/SGFP4-PIVOT-ANALYSIS.md`
  and `.planning/quick/260825-backfill-sgfp4-pivot-phase2-completion/` — recent related
  quick-task history worth reading directly.
- GNUS-NEO-SWARM: `src/core/fp4/fp4_codec.{cpp,hpp}`, `src/core/sgprocessing/{sg_processing_bridge,tensor_interpreter}.{cpp,hpp}`,
  `test/core/test_fp4_codec.cpp`, `test/integration/test_sgprocessing_pipeline.cpp`.
- Recent GNUS-NEO-SWARM history (per top-level git log) includes "SGProcessing bridge fixes
  and test vendoring" (commit d229193) — worth checking whether this already accounts for
  MNN's current sgfp4-pivot state.

</specifics>

<canonical_refs>
## Canonical References

- `W:\gnus\GeniusNetwork\thirdparty\MNN\.planning\sgfp4-arxiv-v2.txt` (and `.pdf`) — the FP4 spec MNN's sgfp4-pivot implements.
- `W:\gnus\GeniusNetwork\thirdparty\MNN\.planning\workstreams\sgfp4-pivot\{REQUIREMENTS,ROADMAP,STATE}.md`
- `GeniusCognitiveSystem\.planning\workstreams\neo\config.json` — pattern to follow if/when a new workstream is scaffolded later.

</canonical_refs>
