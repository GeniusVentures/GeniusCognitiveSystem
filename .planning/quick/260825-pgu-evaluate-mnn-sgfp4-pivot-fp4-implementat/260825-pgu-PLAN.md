---
phase: quick-260825-pgu
plan: 01
type: execute
wave: 1
depends_on: []
files_modified:
  - .planning/quick/260825-pgu-evaluate-mnn-sgfp4-pivot-fp4-implementat/260825-pgu-EVALUATION.md
autonomous: true
requirements:
  - QUICK-260825-pgu-evaluate-mnn-sgfp4-pivot
must_haves:
  truths:
    - "EVALUATION.md exists and delivers a decision-oriented synthesis (not a copy) of RESEARCH.md."
    - "The report leads with the three-FP4-formats clarification and the 'SGFP4 is weight-compression, not an input format' distinction."
    - "The report gives an explicit MNN-side quality verdict, GNUS-NEO-SWARM compatibility status, and SGProcessingManager verification-layer implications."
    - "The report gives an explicit CONDITIONAL yes/no workstream recommendation and, if warranted, sites it as a new top-level GeniusCognitiveSystem/.planning/workstreams/ entry mirroring the neo config.json pattern — as a recommendation only."
    - "The report lists concrete follow-up items (dead-code cleanup, stale-test fixes, ownership reconciliation) without executing any of them."
  artifacts:
    - ".planning/quick/260825-pgu-evaluate-mnn-sgfp4-pivot-fp4-implementat/260825-pgu-EVALUATION.md"
  key_links:
    - "Recommendation is grounded in the three-formats distinction and the weight-compression-vs-input-format finding, not in version drift."
    - "No new workstream/config.json/ROADMAP files are created; recommendation is prose only."
---

<objective>
Synthesize the completed research (`260825-pgu-RESEARCH.md`) into a single, decision-oriented
evaluation document at `260825-pgu-EVALUATION.md`. The evaluation assesses (a) how well MNN's
`sgfp4-pivot` workstream was executed, (b) whether GNUS-NEO-SWARM's FP4 codec / SGProcessing
verification layer are compatible with MNN's current SGFP4 output, and (c) whether a new
GeniusCognitiveSystem workstream is warranted to close the gap.

Purpose: Give the developer a clear, evidence-backed verdict and an actionable conditional
recommendation, so the decision on whether to open a new workstream can be made without
re-reading two full codebases.

Output: `.planning/quick/260825-pgu-evaluate-mnn-sgfp4-pivot-fp4-implementat/260825-pgu-EVALUATION.md`

This is a WRITE-ONLY task. Per the locked CONTEXT.md decision it MUST NOT create or scaffold a
new workstream (no `.planning/workstreams/` entry, no `config.json`, no `ROADMAP.md`) — a
workstream may only be *recommended* in the report. It MUST NOT make any code changes to
GNUS-NEO-SWARM or MNN (no deleting orphaned `fp4_codec`, no fixing stale tests) — those are
follow-up items to be *listed* as recommendations, not executed.
</objective>

<execution_context>
@$HOME/.claude/gsd-core/workflows/execute-plan.md
</execution_context>

<context>
@.planning/quick/260825-pgu-evaluate-mnn-sgfp4-pivot-fp4-implementat/260825-pgu-CONTEXT.md
@.planning/quick/260825-pgu-evaluate-mnn-sgfp4-pivot-fp4-implementat/260825-pgu-RESEARCH.md

# The research is complete and evidence-based (both codebases read in full, concrete
# file:line evidence). Do NOT re-open the codebases to redo research — synthesize from
# RESEARCH.md. Consult a source file only to confirm a single load-bearing quote if needed.
</context>

<tasks>

<task type="auto">
  <name>Task 1: Write the SGFP4 integration evaluation report</name>
  <files>.planning/quick/260825-pgu-evaluate-mnn-sgfp4-pivot-fp4-implementat/260825-pgu-EVALUATION.md</files>
  <action>
Write a decision-oriented evaluation that synthesizes (does not copy) `260825-pgu-RESEARCH.md`
into a document a decision-maker can act on. Preserve the concrete file:line evidence from the
research as support, but restructure around decisions rather than around the research's Q1-Q5
question order.

Use this section structure (exact H2 headings, since the verify step greps for them):

1. `# SGFP4 Integration Evaluation` — title + one-line framing (date 2026-08-25).

2. `## Verdict at a Glance` — a 4-6 bullet TL;DR. Must state up front: MNN sgfp4-pivot is
   high-quality and complete; GNUS-NEO-SWARM is NOT compatible and does not reference SGFP4;
   the codec in GNUS-NEO-SWARM is orphaned NF4 dead code; and the workstream recommendation is
   CONDITIONAL on a scoping decision (defined below).

3. `## The Three FP4 Formats` — LEAD the technical body with this. Explain the three distinct,
   mutually-incompatible "FP4" formats and why conflating them is the central risk:
   (a) GNUS-NEO-SWARM `fp4_codec` NF4-style non-uniform LUT (orphaned dead code);
   (b) MNN E2M1 "Ultra FP4" behind `InputFormat::FP4_ULTRA` (the only live cross-repo FP4 path);
   (c) MNN SGFP4 v2 affine dual-mode (FP4_AFFINE + T158 ternary) quadtree from the arxiv spec.
   Include the key distinction that reframes the whole evaluation: SGFP4 is a MODEL-WEIGHT
   compression format decoded inside MNN's graph via the `OpType_SGFP4Dequant` sidecar op — NOT
   an input-tensor decode path like `dequant_fp4_packed_cpu`. State that most of the apparent
   integration gap dissolves or refocuses once this is understood. Use the comparison table from
   RESEARCH.md Q2 (code space / reconstruction / scale granularity / nibble order / container /
   dual-mode) to make the incompatibility concrete.

4. `## MNN-Side Implementation Quality` — deliver the verdict: HIGH quality, genuinely complete
   for its locked v2-only scope. Summarize the phase-completion evidence (all 4 phases in real
   source with tests; Phase 4 verified on live RTX 4070 Ti SUPER hardware), fidelity to
   `sgfp4-arxiv-v2.txt` (affine w = S·c + bias, FP16 packed S+bias, self-framed 'SGF4' stream,
   5 uniform layouts + MIXED quadtree, bounds-checked), and test coverage. Then list the minor
   red flags as a bulleted list, each labeled as doc-hygiene vs. deferred-perf vs. test-breadth:
   stale REQUIREMENTS.md checkboxes, missing 02-VERIFICATION.md, the unrelated FP4ModelTest.cpp
   build blocker, single true LAYOUT_MIXED GPU fixture, and the correctness-first (not perf-tuned)
   Vulkan per-thread re-walk. Make explicit that NONE of these block correctness.

5. `## GNUS-NEO-SWARM Compatibility` — verdict: NOT compatible and not trying to be. Explain that
   `fp4_codec` is a third, unrelated NF4 format (the exact format the SGFP4 paper explicitly
   rejects), that it is self-flagged orphaned dead code (D-13; consumer removed in commit
   `8ee7fa4`), and that a grep for `sgfp4|affine|quadtree|t158` across GNUS-NEO-SWARM `src`
   returns nothing. Conclude that the incompatibility is architectural (different integration
   surface), not version drift.

6. `## SGProcessingManager Verification-Layer Implications` — explain that the verification layer
   lives in SuperGenius/SGProcessingManager (not in the GNUS-NEO-SWARM files read), that the
   bridge is a thin client and TensorInterpreter only ever sees already-dequantized FLOAT32, and
   that there is NO silent-mis-verify risk today because no `InputFormat::SGFP4` enum exists — an
   SGFP4 job simply cannot be expressed and would be rejected at the format gate. Surface the two
   unresolved / flagged items honestly: (a) the FP4_ULTRA live-decode-vs-stub ambiguity (MEDIUM
   confidence, SuperGenius repo not read this session) and (b) the stale, self-contradicting
   `test_sg_connectivity.cpp` fp4_ultra assertion (static inference, not executed). Note SGFP4 is
   a separate concern from FP4_ULTRA — resolving the E2M1 wiring does NOT address SGFP4.

7. `## Integration Gaps` — present the two conditional gap sets from RESEARCH.md Q4 as clearly
   separated lists: "If SGFP4 stays a model-weight concern (likely correct reading)" (MNN build
   must include `OpType_SGFP4Dequant` + `MNN_SUPPORT_TRANSFORMER_FUSE`; model export to SGFP4
   sidecars belongs to gnus-poc; bridge/interpreter need little-to-no change; orphaned `fp4_codec`
   should be deleted) vs. "Additionally, if GCS/SuperGenius must speak SGFP4 as an input/wire
   format" (new `InputFormat::SGFP4_V2`, new format-map cases, new ProcessingManager/MNN_Tensor
   validation branch, and the arxiv §8 verifiable-execution/attestation story which has no anchor
   anywhere yet). Also call out the ownership-reconciliation gap: GCS `neoswarm/ROADMAP.md:173`
   still assigns SGFP4 GPU decode shaders to GNUS-NEO-SWARM, but MNN already built them — that
   roadmap row is now duplicative/obsolete and needs an explicit decision.

8. `## Workstream Recommendation` — give an EXPLICIT CONDITIONAL recommendation, not an
   unconditional one. State the condition as a scoping decision the developer must make:
   - If the goal is "run SGFP4-quantized models through MNN via the existing SGProcessing path":
     a dedicated workstream is NOT warranted — it is a handful of quick tasks (confirm/bump the
     linked MNN build to include `OpType_SGFP4Dequant`; add SGFP4 export in gnus-poc; one
     end-to-end SGFP4-weighted-model test; a cleanup task for the orphaned NF4 codec + two stale
     tests).
     - If the goal is the full cross-repo story (GCS/SuperGenius speaking SGFP4 as a first-class
     format and/or wiring the arxiv §8 verifiable-execution/attestation use case): YES, a
     dedicated workstream IS warranted — it spans three repos with real sequencing/dependencies
     and touches the reputation/consensus verification layer.
   Include the non-binding P1-P4 phase sketch from RESEARCH.md Q5 for the full-workstream case.
   Then state the SITING recommendation explicitly and as a recommendation ONLY: if/when created
   later, it should live as a new top-level `GeniusCognitiveSystem/.planning/workstreams/` entry
   (sibling to `neo`/`neo-poc`), pointing at the GNUS-NEO-SWARM submodule the same way
   `.planning/workstreams/neo/config.json` does (submodule + planning_root + workstream_name),
   NOT nested inside `GNUS-NEO-SWARM/.planning/workstreams/`. Add an explicit sentence stating
   this task does NOT create that workstream and only recommends it.

9. `## Recommended Follow-Ups` — a checklist of concrete, listed-not-executed items: delete
   orphaned `fp4_codec.{hpp,cpp}` + `test/core/test_fp4_codec.cpp`; fix the stale
   `test_sg_connectivity.cpp` fp4_ultra assertion; reconcile the ROADMAP SGFP4-shader ownership
   row; resolve the FP4_ULTRA live-vs-stub ambiguity against the SuperGenius repo; MNN-side doc
   hygiene (REQUIREMENTS.md checkboxes, missing 02-VERIFICATION.md). Prefix the section with one
   sentence stating these are recommendations for future work and are intentionally NOT performed
   in this evaluation task.

10. `## Confidence and Caveats` — carry forward the research's confidence posture: HIGH on both
    codebases' source and the three-formats finding; MEDIUM on SuperGenius/SGProcessingManager
    internals (repo not read this session); the FP4_ULTRA live-vs-stub ambiguity is unresolved;
    the stale-test failure is a static inference; no builds or tests were run.

Writing constraints:
- Synthesize and reorganize around decisions; do not paste RESEARCH.md verbatim. Keep concrete
  file:line evidence as inline support where it strengthens a claim.
- Do NOT use scope-reduction language (no "v1/v2 simplification", "for now", "placeholder") when
  describing the recommendation — split the recommendation by scope condition as instructed above.
- Honor CONTEXT.md locked decisions: report only; recommend the workstream, do not create it;
  list follow-ups, do not execute them. Do not touch any file other than the EVALUATION.md.
  </action>
  <verify>
    <automated>f=".planning/quick/260825-pgu-evaluate-mnn-sgfp4-pivot-fp4-implementat/260825-pgu-EVALUATION.md"; test -f "$f" || { echo "MISSING FILE"; exit 1; }; for h in "The Three FP4 Formats" "MNN-Side Implementation Quality" "GNUS-NEO-SWARM Compatibility" "Verification-Layer Implications" "Integration Gaps" "Workstream Recommendation" "Recommended Follow-Ups"; do grep -q "$h" "$f" || { echo "MISSING HEADING: $h"; exit 1; }; done; grep -qi "conditional" "$f" || { echo "MISSING conditional recommendation"; exit 1; }; grep -q "workstreams/neo" "$f" || { echo "MISSING neo config siting reference"; exit 1; }; test ! -e .planning/workstreams/sgfp4 || { echo "ERROR: workstream was scaffolded (forbidden)"; exit 1; }; echo "OK"</automated>
  </verify>
  <done>
`260825-pgu-EVALUATION.md` exists with all ten required sections; leads with the three-formats
clarification and the weight-compression-vs-input-format distinction; states the MNN quality
verdict (HIGH/complete), the GNUS-NEO-SWARM incompatibility (orphaned NF4 dead code), the
verification-layer implications, the two conditional gap sets, and an explicit CONDITIONAL
workstream recommendation with the top-level `.planning/workstreams/` siting stated as a
recommendation only. No workstream files, config.json, ROADMAP.md, or code changes were created.
  </done>
</task>

</tasks>

<verification>
- `260825-pgu-EVALUATION.md` exists and contains all required H2 sections (automated grep).
- Recommendation is explicitly conditional and cites the neo `config.json` siting pattern.
- No new `.planning/workstreams/` entry, `config.json`, or `ROADMAP.md` was created.
- No source files in GNUS-NEO-SWARM or MNN were modified (git status shows only the new
  EVALUATION.md under the quick-task directory).

Note: No `<threat_model>` — this plan produces a single Markdown document with no code, no
package installs, and no runtime attack surface, so STRIDE analysis is not applicable.
</verification>

<success_criteria>
- A single evaluation document synthesizes the research into an actionable, decision-oriented
  verdict covering all five required dimensions plus a conditional workstream recommendation.
- The locked CONTEXT.md decisions are honored: report only, recommend-don't-create the
  workstream, list-don't-execute the follow-ups.
</success_criteria>

<output>
Write `.planning/quick/260825-pgu-evaluate-mnn-sgfp4-pivot-fp4-implementat/260825-pgu-EVALUATION.md`.
(No SUMMARY file required for this quick task unless the quick orchestrator requests one.)
</output>
