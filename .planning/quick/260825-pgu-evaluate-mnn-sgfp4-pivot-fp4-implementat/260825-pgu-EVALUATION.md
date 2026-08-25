# SGFP4 Integration Evaluation

Evaluation of MNN's `sgfp4-pivot` workstream against GNUS-NEO-SWARM's FP4 codec and the
SGProcessingManager verification layer, and a conditional recommendation on whether a new
GeniusCognitiveSystem workstream is warranted to close the gap. (2026-08-25)

## Verdict at a Glance

- **MNN's `sgfp4-pivot` is high-quality and genuinely complete** for its locked v2-only scope:
  all 4 phases exist in real source with real tests, and Phase 4 was verified on live GPU
  hardware (RTX 4070 Ti SUPER).
- **GNUS-NEO-SWARM is NOT compatible with MNN's SGFP4** and does not reference it anywhere —
  not because of version drift, but because it is architecturally not built to.
- **The FP4 codec already inside GNUS-NEO-SWARM (`fp4_codec.{hpp,cpp}`) is orphaned NF4 dead
  code** — a third, unrelated format that the SGFP4 spec itself explicitly rejects as a design.
- Most of the apparent "integration gap" dissolves once one fact is understood: **SGFP4 is a
  model-weight compression format decoded inside MNN's graph, not an input-tensor format** like
  the `FP4_ULTRA` path GNUS-NEO-SWARM currently speaks.
- **The workstream recommendation is CONDITIONAL** on a scoping decision the developer must
  make (see "Workstream Recommendation" below) — it is not a blanket yes or no.
- No code was changed and no workstream was scaffolded as part of producing this report; all
  actions below are recommendations only.

## The Three FP4 Formats

The single biggest risk in reasoning about this integration is conflating three distinct,
mutually-incompatible things that all happen to be called "FP4":

1. **GNUS-NEO-SWARM `fp4_codec`** (`GNUS-NEO-SWARM/src/core/fp4/fp4_codec.{hpp,cpp}`) — an
   NF4-style non-uniform 16-value lookup-table codec (QLoRA NF4 lineage). It is orphaned dead
   code today (see "GNUS-NEO-SWARM Compatibility" below).
2. **MNN E2M1 "Ultra FP4"** (`include/MNN/FP4DequantUtils.hpp::dequant_fp4_packed_cpu`), reached
   through `InputFormat::FP4_ULTRA`. This is the **only live cross-repo FP4 path today**: GCS →
   `SGProcessingBridge` (labels the job `FP4_ULTRA`) → SGProcessingManager → MNN's E2M1 decode.
3. **MNN SGFP4 v2** (the `sgfp4-pivot` workstream's subject, new `OpType_SGFP4Dequant`) — the
   affine-integer dual-mode (FP4_AFFINE + T158 ternary) quadtree container defined in
   `sgfp4-arxiv-v2.txt`. Complete on CPU + Vulkan, well-tested, verified on live GPU hardware —
   but consumed by nothing on the GNUS-NEO-SWARM / GCS / SuperGenius side.

The distinction that reframes the whole evaluation: **SGFP4 is a model-weight compression
format, decoded inside MNN's graph via the external sidecar op `OpType_SGFP4Dequant`** — it is
not an input-tensor decode path like `dequant_fp4_packed_cpu`. Consuming SGFP4 most likely means
*running a model whose weights were exported to SGFP4*, not *feeding SGFP4 bytes through the
FP4_ULTRA input slot*. Once that is understood, most of the apparent integration gap either
dissolves (no bridge/interpreter changes needed) or refocuses onto a much narrower question
(does the linked MNN build register the op, and does something export SGFP4 weights).

| Dimension | GNUS-NEO-SWARM `fp4_codec` | MNN SGFP4 v2 | MNN E2M1 "Ultra" |
|-----------|---------------------------|--------------|------------------|
| Code space | NF4 non-uniform 16-value LUT `kFP4LUT[16]` (`fp4_codec.hpp:32-33`) | Affine integer: FP4 `[-8,7]` + ternary `{-1,0,1}` | E2M1 float (±0.5…±3, Inf/NaN) |
| Reconstruction | `LUT[idx] * scale` (`fp4_codec.cpp:53-56`) | `S·c + bias` (real FP16 bias) | table lookup, per-channel scale, bias≡0 |
| Scale granularity | 1 FP32 scale per 64×64 macroblock (`fp4_codec.cpp:124`) | FP16 S+bias per leaf (down to 4×4) | per-output-channel FP32 (symmetricQuan) |
| Nibble order | high nibble = even index (`fp4_codec.cpp:156-159`) | code i at bit `4·(i mod 8)` → low nibble = index 0 | low nibble = even index |
| Container | flat `data_/scales_` arrays, no header | self-framed `'SGF4'` stream, quadtree | MNN `symmetricQuan` flat arrays |
| Dual-mode / ternary | none | core feature | none |

`sgfp4-arxiv-v2.txt:73-76` explicitly contrasts SGFP4 against NF4: "QLoRA's NF4 uses a 16-entry
non-uniform codebook… SGFP4 deliberately uses affine integer codes instead." GNUS-NEO-SWARM's
`fp4_codec` **is** the NF4 approach the spec deliberately rejects — even the nibble order is
inverted, so the packed bytes of the two formats are not interchangeable at any level.

## MNN-Side Implementation Quality

**Verdict: HIGH quality, genuinely complete for its locked v2-only scope.**

All 4 phases are implemented in real source (not just planning docs), backed by real tests, and
Phase 4 was independently verified on live GPU hardware:

| Phase | Status | Code evidence |
|-------|--------|----------------|
| 1 — Affine dual-mode CPU, uniform layouts | Complete (2026-08-24) | `include/MNN/SGFP4DequantUtils.hpp` (460 lines), `source/backend/cpu/CPUSGFP4Dequant.{cpp,hpp}`, registered in `CPUOPRegister.cpp`/`ShapeRegister.cpp` |
| 2 — Adaptive quadtree CPU, MIXED layout | Complete (2026-08-24) | `sgfp4_walk_quadtree` + MIXED branch (`SGFP4DequantUtils.hpp:188-222, 387-408`); encoder `tools/fp4/encode_sgfp4.py` (1085 lines) |
| 3 — Vulkan uniform layouts | Complete (2026-08-25) | `sgfp4_dequant.comp`, `VulkanSGFP4Dequant.{cpp,hpp}`, regenerated shader maps |
| 4 — Vulkan MIXED layout | Complete (2026-08-25) | `locateMixedLeaf` (`sgfp4_dequant.comp:114-180`); `04-VERIFICATION.md` reports a live run on an RTX 4070 Ti SUPER, 14 fixtures green |

Fidelity to `sgfp4-arxiv-v2.txt` is close and section-by-section: the affine reconstruction
`w = S·c + bias` for both FP4_AFFINE and T158 ternary modes matches spec §3.2/§4.3 Eq.3/4
exactly (including the reserved-symbol rule for ternary code `11`); the FP16 packed S+bias
header with 12-bit truncated-bias recovery matches §6.2 Eq.6 and uses the vendored
`half_float::half` rather than a hand-rolled implementation; the self-framed `'SGF4'` stream
(magic/version/16-byte-aligned record-offset table) matches §6.1; all 5 uniform layouts plus the
LAYOUT_MIXED quadtree pre-order DFS walk match §6.2 Table 3; every read is bounds-checked and
rejects malformed containers rather than reading out of bounds. The Vulkan shader is a
documented bit-for-bit port of the CPU walker, cross-checked line-by-line against the CPU split-
map reader in `04-VERIFICATION.md`. Test coverage is real: `test/op/SGFP4DequantTest.cpp` (866
lines, two independent suites — one of which shares no code with the decoder, a deliberate
cross-check design decision), `test/op/SGFP4VulkanDequantTest.cpp` (CPU-vs-Vulkan parity within
rtol), and `tools/fp4/encode_sgfp4.py` (a real reference encoder with its own selftest).

Minor red flags, none of which block correctness:

- **Doc-hygiene:** `REQUIREMENTS.md:25-28` still shows Phase 2 requirements (SGV2-08…11)
  unchecked and the traceability table lists them "Pending," even though the code fully
  implements them — `STATE.md` acknowledges this is stale-doc debt, and a separate backfill
  quick task confirmed via git history that Phase 2 was genuinely executed.
- **Doc-hygiene:** no `02-VERIFICATION.md` exists (Phases 1, 3, 4 each have one); Phase 2
  completion is evidenced only by SUMMARY files and commits.
- **Deferred-perf / unrelated blocker:** `test/op/FP4ModelTest.cpp` blocks a from-scratch
  `run_test.out` build, but it is pre-existing dead code from an unrelated `milestone`
  workstream, not something sgfp4-pivot introduced; verifiers worked around it with a temp stub.
- **Test-breadth:** the 14-fixture Vulkan sweep contains exactly one true LAYOUT_MIXED fixture
  (`mixed_asymmetric`); the quadtree-on-GPU path is exercised but by a single tree shape.
- **Deferred-perf:** Vulkan decode does a full per-thread re-walk of the record/leaf structure
  per output element (O(records × leaves) per element) rather than the spec's "one workgroup per
  macroblock" shared-memory model — correctness-first by deliberate decision (SGV2-18), worth
  revisiting only if throughput becomes a concern.

Deliberately out of scope in MNN (locked decisions, not defects): the SGFP4 v1 fixed-payload
profile (v2-only), attestation / bit-exact conformance vectors (SuperGenius's job), and any
SuperGenius-side integration.

## GNUS-NEO-SWARM Compatibility

**Verdict: NOT compatible, and not even trying to be.**

GNUS-NEO-SWARM's `fp4_codec` is a third, unrelated NF4 format — the exact non-uniform-codebook
approach the SGFP4 paper explicitly argues against (see table above). It is also self-flagged
orphaned dead code: `fp4_codec.hpp:8-13` documents "Flagged per Phase 4 D-13: this NF4-style
codec predates and does not match MNN_Ultra's E2M1 target format for `InputFormat::FP4_ULTRA`.
MNNInferenceEngine's reference to this class was confirmed orphaned and removed (Phase 4 plan
04-04). This class is a candidate for removal." Commit `8ee7fa4` ("fix(04-04): remove orphaned
fp4_codec member from MNNInferenceEngine") performed that removal; `test/core/test_fp4_codec.cpp`
(6 tests) still exists and tests the now-dead class in isolation, but nothing in live `src/`
calls into it.

A grep for `sgfp4|affine|quadtree|t158` across GNUS-NEO-SWARM `src` returns nothing.
`mnn_inference_engine.cpp` has no SGFP4 awareness at all; for FP4 it only ever sets
`input_fmt = FP4_ULTRA` (`:266-267`) and hands off to the bridge, targeting MNN's E2M1 decode —
not SGFP4.

The incompatibility is **architectural** — a different integration surface (model-weight sidecar
vs. input-tensor pass-through) — not version drift that would be fixed by pointing GNUS-NEO-SWARM
at a newer MNN commit.

## SGProcessingManager Verification-Layer Implications

The verification layer itself lives in SuperGenius/SGProcessingManager, which was not part of
the files read this session. On the GNUS-NEO-SWARM side, `SGProcessingBridge`
(`sg_processing_bridge.cpp`) is a thin client: it builds GNUS-schema JSON and submits a job
(`SubmitDirect` → `ProcessingManager::Create`/`Process`, `:308-380`); it performs no FP4 decode
and no verification itself. `TensorInterpreter` operates only on the output, which is already
dequantized to FLOAT32 by the time it returns (`tensor_interpreter.cpp:54-56`); it never sees a
raw FP4/SGFP4 container, and `InferViaSGProcessing` calls `Interpret(..., FLOAT32)`
unconditionally (`mnn_inference_engine.cpp:275`).

**There is no silent-mis-verify risk today**, because no `InputFormat::SGFP4` (or `SGFP4_V2`)
enum exists anywhere in the format maps (`InputFormatToTypeString` /
`InputFormatToFormatString`, `sg_processing_bridge.cpp:40-95`, only know FLOAT32/16, INT8/16/32,
RGB(A)8, FP4_ULTRA). An SGFP4 job simply cannot be expressed today — it would be rejected at the
format gate rather than silently mis-decoded or mis-verified.

Two items are surfaced honestly as unresolved rather than glossed over:

1. **The FP4_ULTRA live-decode-vs-stub ambiguity (MEDIUM confidence; SuperGenius repo not read
   this session).** MNN-side analysis (`SGFP4-PIVOT-ANALYSIS.md:126-142`) claims FP4_ULTRA
   decode is wired live in SuperGenius (`processing_processor_mnn_tensor.cpp` calling
   `MNN::dequant_fp4_packed_cpu`, commit `e1f28d7`) and flags a stale SuperGenius test asserting
   the old unsupported-format behavior. GNUS-NEO-SWARM's own Phase 04 plan
   (`04-02-PLAN.md:19-31`) describes FP4_ULTRA as "wire + stub" returning
   `FORMAT_UNSUPPORTED`, from a point when MNN's E2M1 kernel was 39 commits diverged. These two
   accounts are from different dates/branches and could not be reconciled without reading the
   SuperGenius repo directly.
2. **The stale, self-contradicting `test_sg_connectivity.cpp` fp4_ultra assertion** (static
   inference, not executed): `test_sg_connectivity.cpp:63-72` asserts the schema JSON contains
   the lowercase literal `"fp4_ultra"`, but Phase 04-01 (`83a0e84`) changed the bridge so only
   the uppercase `format` field carries `"FP4_ULTRA"` — this directly contradicts
   `test_sgprocessing_pipeline.cpp:96`, which asserts the lowercase literal is absent.

Both of these concern the E2M1/FP4_ULTRA path specifically. **SGFP4 is a separate concern from
FP4_ULTRA** — resolving the E2M1 live-vs-stub ambiguity does not address SGFP4 in any way, since
no SGFP4 code path exists on the GNUS-NEO-SWARM/SuperGenius side to resolve.

## Integration Gaps

The key clarification from "The Three FP4 Formats" above splits the gap list into two
conditional sets, depending on a scoping decision that has not yet been made.

**If SGFP4 stays a model-weight concern (the likely correct reading):**

- MNN submodule/build must include `OpType_SGFP4Dequant` and be built with
  `MNN_SUPPORT_TRANSFORMER_FUSE` (the op and its tests are gated on this macro) — confirm the
  MNN build GNUS-NEO-SWARM links against has this registered.
- Model export to SGFP4 sidecars belongs to `gnus-poc` per the existing GCS ROADMAP row
  ("SGFP4 quantization export | gnus-poc | poc (Python)," `neoswarm/ROADMAP.md:161`);
  `tools/fp4/encode_sgfp4.py` is the reference encoder to build on.
- `SGProcessingBridge` / `TensorInterpreter` need little to no change — output is still FLOAT32,
  and the SGFP4 container lives with the model rather than the input tensor.
- The orphaned `fp4_codec.{hpp,cpp}` and its test should be deleted.

**Additionally, if GCS/SuperGenius must speak SGFP4 as an input/wire format** (only if that
turns out to be a real requirement):

- A new `InputFormat::SGFP4_V2` in SuperGenius `generated/InputFormat.hpp`.
- New cases in `InputFormatToTypeString` / `InputFormatToFormatString`
  (`sg_processing_bridge.cpp:40-95`).
- A new SGFP4 validation branch in SGProcessingManager's `ProcessingManager::CheckProcessValidity`
  and the `MNN_Tensor` processor (currently only FP4_ULTRA/E2M1).
- The arxiv §8 verifiable-execution/attestation story (bit-exact cross-device replay, ternary
  integer-exact path) — this currently has **no anchor anywhere** in the codebase family and
  would be net-new work in both MNN and SuperGenius if pursued.

**Ownership-reconciliation gap:** GCS `neoswarm/ROADMAP.md:173` still assigns "SGFP4 GPU decode
shaders (Vulkan/MoltenVK)" to GNUS-NEO-SWARM — but MNN's sgfp4-pivot has already built exactly
that inside MNN (`sgfp4_dequant.comp`). That roadmap row is now duplicative/obsolete and needs
an explicit decision: does GNUS-NEO-SWARM consume MNN's SGFP4 Vulkan decode, or was that
ownership row superseded by MNN owning the decode outright?

## Workstream Recommendation

**CONDITIONAL — the answer depends on a scoping decision the developer needs to make, not on
anything ambiguous in the evidence.**

- **If the goal is "run SGFP4-quantized models through MNN via the existing SGProcessing
  path":** a dedicated workstream is **NOT** warranted. This is a handful of quick tasks:
  confirm/bump the linked MNN build to include `OpType_SGFP4Dequant`; add SGFP4 export in
  `gnus-poc`; write one end-to-end test running an SGFP4-weighted model; and a cleanup task
  deleting the orphaned NF4 `fp4_codec` plus fixing the two stale FP4-related tests. The bridge
  and interpreter are essentially unchanged, since decode stays internal to MNN and the output
  is still FLOAT32.

- **If the goal is the full cross-repo story** — GCS/SuperGenius speaking SGFP4 as a first-class
  format and/or wiring the arxiv §8 verifiable-execution/attestation use case — **YES, a
  dedicated workstream IS warranted.** It spans three repos (`gnus-poc` export, MNN submodule
  integration, SuperGenius `InputFormat` + processor + verification), has genuine
  sequencing/dependencies, and touches the reputation/consensus verification layer that is the
  paper's core motivation.

Non-binding phase sketch for the full-workstream case, for reference if that path is chosen:

- **P1 — Reconcile & clean up:** delete orphaned NF4 `fp4_codec` + its test; fix stale
  `test_sg_connectivity.cpp` and (cross-repo) the stale SuperGenius test; resolve the ROADMAP
  ownership row (MNN owns SGFP4 decode vs. GNUS-NEO-SWARM shaders); pin the MNN submodule to a
  commit that includes `OpType_SGFP4Dequant`.
- **P2 — SGFP4 model export + MNN graph consumption:** `gnus-poc` exports weights to SGFP4
  sidecars (building on `encode_sgfp4.py`); end-to-end inference of an SGFP4-weighted model
  through MNN LLM / SGProcessing; output-parity check.
- **P3 (only if SGFP4-as-input is actually required):** SuperGenius `InputFormat::SGFP4_V2` +
  bridge mapping + `ProcessingManager`/`MNN_Tensor` validation branch.
- **P4 (optional, arxiv §8):** verifiable-execution/attestation — bit-exact replay and
  ternary integer-exact determinism class; net-new in both MNN and SuperGenius.

**Siting recommendation (recommendation only — this task does NOT create it):** if and when a
new workstream is created later, it should live as a new top-level
`GeniusCognitiveSystem/.planning/workstreams/` entry, sibling to the existing `neo`/`neo-poc`
entries, pointing at the GNUS-NEO-SWARM submodule the same way `.planning/workstreams/neo/config.json`
does — submodule reference, `planning_root`, and `workstream_name` — **not** nested inside
`GNUS-NEO-SWARM/.planning/workstreams/`. This evaluation task does not create that workstream,
its `config.json`, or any ROADMAP entry; it only recommends the siting for future use.

## Recommended Follow-Ups

The items below are recommendations for future work. They are intentionally **not** performed as
part of this evaluation task.

- [ ] Delete the orphaned `fp4_codec.{hpp,cpp}` and `test/core/test_fp4_codec.cpp` from
      GNUS-NEO-SWARM (self-flagged dead code, consumer already removed in commit `8ee7fa4`).
- [ ] Fix the stale `test_sg_connectivity.cpp` fp4_ultra assertion, which contradicts the
      already-updated `test_sgprocessing_pipeline.cpp`.
- [ ] Reconcile the `neoswarm/ROADMAP.md:173` row that still assigns SGFP4 GPU decode shaders to
      GNUS-NEO-SWARM, now that MNN owns that decode.
- [ ] Resolve the FP4_ULTRA live-vs-stub ambiguity by reading the SuperGenius repo directly
      (separate from and does not resolve the SGFP4 question).
- [ ] MNN-side doc hygiene: check off the stale Phase 2 REQUIREMENTS.md boxes and update the
      traceability table; add the missing `02-VERIFICATION.md`.

## Confidence and Caveats

- **HIGH** on both codebases' source (both read in full this session), on MNN's phase-completion
  evidence, on the three-formats finding, and on `fp4_codec` being orphaned dead code.
- **MEDIUM** on SuperGenius/SGProcessingManager internals — the SuperGenius repo itself was not
  read this session; findings about it are secondhand via MNN-side analysis docs and
  GNUS-NEO-SWARM plan documents.
- The FP4_ULTRA live-decode-vs-stub ambiguity is **unresolved** and would require reading the
  SuperGenius repo to settle.
- The stale `test_sg_connectivity.cpp` failure is a **static inference**, not a result of
  actually running the test.
- **No builds or tests were run** as part of this research or this evaluation.
