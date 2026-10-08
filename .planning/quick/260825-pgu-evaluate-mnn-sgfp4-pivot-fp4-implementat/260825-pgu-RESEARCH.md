# Quick Task 260825-pgu — Research: MNN sgfp4-pivot ⇄ GNUS-NEO-SWARM FP4 / SGProcessing integration

**Researched:** 2026-08-25
**Domain:** FP4/SGFP4 weight-quantization decode; MNN Execution ops; GNUS-NEO-SWARM ⇄ SGProcessingManager bridge
**Confidence:** HIGH on both codebases' source (read in full); MEDIUM on SuperGenius/SGProcessingManager internals (read only via the MNN-side analysis excerpts and NEO-SWARM plan docs — the SuperGenius repo itself was not in the read set). No builds or tests were run this session; a few findings are static inferences and are flagged as such.

---

## Executive Summary

There are **three distinct, mutually-incompatible "FP4" formats** in this repo family, and conflating them is the central risk this evaluation must clear up:

1. **GNUS-NEO-SWARM `fp4_codec`** (`GNUS-NEO-SWARM/src/core/fp4/fp4_codec.{hpp,cpp}`) — an **NF4-style non-uniform 16-value lookup-table** codec (QLoRA NF4 lineage). Per-64×64-macroblock symmetric scale, **no bias**, high-nibble = even index. It is **orphaned dead code**: its own header self-flags it (D-13), and its only consumer reference was removed from `MNNInferenceEngine` (commit `8ee7fa4`). It is **not** SGFP4 and **not** MNN's E2M1.
2. **MNN E2M1 "Ultra FP4"** (`include/MNN/FP4DequantUtils.hpp::dequant_fp4_packed_cpu`) — an E2M1 float microformat behind `InputFormat::FP4_ULTRA`. This is the **only live cross-repo FP4 path today**: GCS → `SGProcessingBridge` (FP4_ULTRA) → SGProcessingManager → MNN's E2M1 decode.
3. **MNN SGFP4 v2** (the `sgfp4-pivot` workstream, new `OpType_SGFP4Dequant`) — the affine-integer dual-mode (FP4_AFFINE + T158 ternary) quadtree container from `sgfp4-arxiv-v2.txt`. **Complete on CPU + Vulkan, well-tested, verified on live GPU hardware — but consumed by nothing on the GNUS-NEO-SWARM / GCS / SuperGenius side.**

**Primary conclusion:** MNN's sgfp4-pivot is a high-quality, genuinely-complete implementation. GNUS-NEO-SWARM is **not** compatible with it and does not reference it in any way — not because of a version drift, but because SGFP4 is a **model-weight compression format decoded inside MNN's graph via an external sidecar op**, which is a fundamentally different integration surface than the **input-tensor** `FP4_ULTRA` pass-through that GNUS-NEO-SWARM's bridge currently speaks. The integration gap is real but its *shape* depends on a prerequisite architecture decision (is SGFP4 a model-weight concern owned by MNN + the export pipeline, or does GCS/SuperGenius need to speak it directly?). A dedicated workstream is warranted **only** for the full cross-repo story; the minimal "run SGFP4-quantized models through MNN" path is closer to a few quick tasks plus a dead-code cleanup.

---

## Q1 — MNN-side implementation quality

**Verdict: HIGH quality, genuinely complete for its locked v2-only scope.** All 4 phases are implemented in real source (not just planning docs), with real tests, and Phase 4 was independently verified on live GPU hardware.

### Phase completion — docs vs. actual code

| Phase | Roadmap status | Code evidence | Real? |
|-------|----------------|---------------|-------|
| 1 — Affine dual-mode CPU, uniform | Complete (2026-08-24) | `include/MNN/SGFP4DequantUtils.hpp` (460 lines), `source/backend/cpu/CPUSGFP4Dequant.{cpp,hpp}`, `source/shape/ShapeSGFP4Dequant.cpp`, registered in `CPUOPRegister.cpp` / `ShapeRegister.cpp` | Yes |
| 2 — Adaptive quadtree CPU, MIXED | Complete (2026-08-24) | `sgfp4_walk_quadtree` + MIXED branch in `dequant_sgfp4_container_cpu` (`SGFP4DequantUtils.hpp:188-222, 387-408`); encoder `tools/fp4/encode_sgfp4.py` (1085 lines) | Yes |
| 3 — Vulkan uniform | Complete (2026-08-25) | `source/backend/vulkan/buffer/execution/glsl/sgfp4_dequant.comp`, `VulkanSGFP4Dequant.{cpp,hpp}`, regenerated `AllShader.cpp/.h` + `VulkanShaderMap.cpp` | Yes |
| 4 — Vulkan MIXED | Complete (2026-08-25) | `locateMixedLeaf` in `sgfp4_dequant.comp:114-180`; `04-VERIFICATION.md` reports live run on RTX 4070 Ti SUPER, 14 fixtures green | Yes |

### Fidelity to `sgfp4-arxiv-v2.txt`

The CPU decoder (`SGFP4DequantUtils.hpp`) is a faithful, section-by-section implementation of the normative spec:
- **Affine reconstruction `w = S·c + bias`** for both modes — `sgfp4_decode_leaf_payload` (`:262-282`). Mode 0 FP4_AFFINE two's-complement `[-8,7]` via `(nib ^ 0x8) - 0x8`; Mode 1 T158 ternary `00→0, 01→+1, 10→-1, 11→0(reserved)`. Matches spec §3.2 / §4.3 Eq.3/4 and the reserved-symbol rule exactly.
- **FP16 packed S+bias with the v2 12-bit truncated-bias recovery** `S=half(h>>16)`, `bias=half(h & 0xFFF0)`, `flags=h & 0xF` — `unpack_leaf_header` (`:232-244`), using the vendored `half_float::half` (no hand-rolled FP16). Matches §6.2 Eq.6.
- **Self-framed stream** magic `'SGF4'` / version `0x02` / `B` / 16-byte-aligned record-offset table / `align16(16+4B)` region start — `dequant_sgfp4_container_cpu:330-360`. Matches §6.1.
- **All 5 uniform layouts** (`sgfp4_resolve_uniform_layout:105-135`) + **LAYOUT_MIXED quadtree** pre-order DFS TL/TR/BL/BR split-map walk (`sgfp4_walk_quadtree:188-222`). Matches §6.2 Table 3.
- Bounds-checked on every read (ASVS V5 posture), rejects malformed containers with `false` rather than OOB.

The Vulkan shader (`sgfp4_dequant.comp`) is a documented bit-for-bit port of the CPU walker (constants ported 1:1; `unpackHalf2x16` for FP16). `04-VERIFICATION.md` cross-checked the GLSL split-map read formula against the CPU `SGFP4SplitMapReader` line-by-line.

### Test coverage (real, but note the caveats)
- `test/op/SGFP4DequantTest.cpp` (866 lines) — two suites: `op/sgfp4/uniform_decode` (cross-language fixture round-trip for both modes × all 5 uniform layouts, degenerate MIXED smoke, hand-built TL/TR/BL/BR golden traversal, ternary reserved-symbol, FP16 header precision incl. 0xFFF0 mask, malformed-container negatives, op-level end-to-end via external sidecar) and `op/sgfp4/mixed_decode` (independent golden enumerator that shares no code with the decoder — D-05, mixed round-trip, split-map negative cases).
- `test/op/SGFP4VulkanDequantTest.cpp` (190 lines) — `op/sgfp4/vulkan_uniform_parity`: CPU-reference-vs-Vulkan for all fixtures within rtol, graceful skip when no Vulkan device.
- `tools/fp4/encode_sgfp4.py` — real error-driven quadtree encoder (`subdivide_macroblock`, `build_split_map`, `classify_layout`, `apply_ternary_veto`, mode-select ε=0.10), a reference decoder, a `selftest()`, and a C++ fixture emitter.

### Red flags (all minor / documentation-level, none blocking correctness)
1. **Stale REQUIREMENTS.md checkboxes.** `REQUIREMENTS.md:25-28` shows SGV2-08…11 (Phase 2 CPU quadtree) *unchecked* and the bottom traceability table (`:73-76`) lists them "Pending" — yet the code fully implements them. `STATE.md:52-55` explicitly acknowledges this is stale-doc debt, and the `260825-backfill-sgfp4-pivot-phase2-completion` quick task confirmed via git history (commits `1c9e5633`/`b2a83969`) that Phase 2 was really executed. Not a code gap; a doc-hygiene gap.
2. **No `02-VERIFICATION.md`** (Phases 1, 3, 4 have one). Phase 2 completion is evidenced only by SUMMARY files + commits. Doc debt.
3. **`test/op/FP4ModelTest.cpp` blocks a from-scratch `run_test.out` build** — pre-existing dead code from the *unrelated* `milestone` workstream (commit `cffaf4bd`), not sgfp4's fault (`deferred-items.md`). Verifiers worked around it with a temp neutral stub; it remains unfixed.
4. **Modest MIXED GPU parity breadth** — the 14-fixture Vulkan sweep contains exactly **one** true LAYOUT_MIXED fixture (`mixed_asymmetric`, `SGFP4DequantFixtures.h:137`). The quadtree-on-GPU path is exercised, but by a single tree shape.
5. **Vulkan decode is correctness-first, not perf-tuned.** `locateElement` does a full **per-thread re-walk** of the record/leaf structure for every output element (O(records × leaves) per element), rather than the spec's "one workgroup per macroblock" shared-memory model. This is functionally correct and deliberately deferred (SGV2-18, a v2 requirement); flag it if throughput ever matters.

**Deliberately out of scope in MNN (locked decisions, not defects):** SGFP4 v1 fixed-payload profile (v2-only); attestation / bit-exact conformance vectors (SuperGenius's job); SuperGenius-side integration.

---

## Q2 — GNUS-NEO-SWARM FP4 codec compatibility

**Verdict: NOT compatible, and not even trying to be — GNUS-NEO-SWARM's `fp4_codec` is a third, unrelated format and is orphaned dead code. Nothing in GNUS-NEO-SWARM calls MNN's sgfp4-pivot decode paths.**

### `fp4_codec` is a different format from BOTH SGFP4 and E2M1

| Dimension | GNUS-NEO-SWARM `fp4_codec` | MNN SGFP4 v2 | MNN E2M1 "Ultra" |
|-----------|---------------------------|--------------|------------------|
| Code space | NF4 non-uniform 16-value LUT `kFP4LUT[16]` (`fp4_codec.hpp:32-33`) | Affine integer: FP4 `[-8,7]` + ternary `{-1,0,1}` | E2M1 float (±0.5…±3, Inf/NaN) |
| Reconstruction | `LUT[idx] * scale` (`fp4_codec.cpp:53-56`) | `S·c + bias` (real FP16 bias) | table lookup, per-channel scale, bias≡0 |
| Scale granularity | 1 FP32 scale per 64×64 macroblock (`fp4_codec.cpp:124`) | FP16 S+bias per leaf (down to 4×4) | per-output-channel FP32 (symmetricQuan) |
| Nibble order | **high nibble = even index** (`fp4_codec.cpp:156-159`) | code i at bit `4·(i mod 8)` → **low nibble = index 0** | low nibble = even index |
| Container | flat `data_/scales_` arrays, no header | self-framed `'SGF4'` stream, quadtree | MNN `symmetricQuan` flat arrays |
| Dual-mode / ternary | none | core feature | none |

The SGFP4 paper (`sgfp4-arxiv-v2.txt:73-76`) *explicitly contrasts itself against NF4*: "QLoRA's NF4 uses a 16-entry non-uniform codebook… SGFP4 deliberately uses affine integer codes instead." GNUS-NEO-SWARM's codec **is** the NF4 approach the spec rejects — even the opposite nibble order means the packed bytes are not interchangeable.

### It is orphaned dead code
`fp4_codec.hpp:8-13` self-documents: *"Flagged per Phase 4 D-13: this NF4-style codec predates and does not match MNN_Ultra's E2M1 target format for InputFormat::FP4_ULTRA. MNNInferenceEngine's reference to this class was confirmed orphaned and removed (Phase 4 plan 04-04). This class is a candidate for removal."* Commit `8ee7fa4` ("fix(04-04): remove orphaned fp4_codec member from MNNInferenceEngine") did the removal. `test/core/test_fp4_codec.cpp` (6 tests) still exists and tests the dead class in isolation. Grep confirms **zero** references to `fp4_codec` from `mnn_inference_engine.cpp` or anywhere else in live `src/`.

### Nothing consumes MNN's SGFP4
Grep for `sgfp4|affine|quadtree|t158` across `GNUS-NEO-SWARM/src` returns **nothing**. `mnn_inference_engine.cpp` has no SGFP4 awareness; for FP4 it only sets `input_fmt = FP4_ULTRA` (`:266-267`) and hands off to the bridge. The one live FP4 path (FP4_ULTRA) targets MNN's **E2M1** `dequant_fp4_packed_cpu`, not SGFP4.

---

## Q3 — SGProcessingManager verification-layer implications

**Verdict: The verification layer is in SuperGenius/SGProcessingManager, not in the GNUS-NEO-SWARM files read here. On the GNUS-NEO-SWARM side there is nothing that would silently mis-verify SGFP4 mixed-layout output — because there is no SGFP4 path at all; an SGFP4 job simply cannot be expressed today.**

### What the GNUS-NEO-SWARM "bridge" side actually does
- `SGProcessingBridge` (`sg_processing_bridge.cpp`) is a **thin client**: it builds GNUS-schema JSON (`BuildSchemaJson`) and submits a job (`SubmitDirect` → `ProcessingManager::Create`/`Process`, `:308-380`). It performs **no** FP4 decode and **no** verification itself. Decode + verification happen inside SGProcessingManager (SuperGenius repo).
- `TensorInterpreter` (`tensor_interpreter.cpp`) operates on the **output** bytes, which are already-dequantized FLOAT32 by the time they return (`:54-56`: *"FP4_ULTRA output from SGProcessingManager is already dequantized to FLOAT32"*). It never sees an FP4/SGFP4 container, so new tensor layouts from adaptive-quadtree decode would not reach it. `InferViaSGProcessing` calls `Interpret(..., FLOAT32)` unconditionally (`mnn_inference_engine.cpp:275`).

### Why there is no "silent mis-verify" risk today
There is **no `InputFormat::SGFP4`/`SGFP4_V2` enum**. `InputFormatToTypeString` / `InputFormatToFormatString` (`sg_processing_bridge.cpp:40-95`) only know FLOAT32/16, INT8/16/32, RGB(A)8, FP4_ULTRA. An SGFP4 job cannot be labeled, so it would be rejected at `DataType::from_json` / the processor format gate rather than mis-verified. Today's whole FP4 story assumes E2M1/FP4_ULTRA.

### Cross-repo verification status is genuinely ambiguous (flag for follow-up)
There is **conflicting evidence** on whether SGProcessingManager currently *decodes* FP4_ULTRA live or still returns a structured stub:
- MNN-side `SGFP4-PIVOT-ANALYSIS.md:126-142` says FP4_ULTRA decode is **wired live** (`processing_processor_mnn_tensor.cpp` calls `MNN::dequant_fp4_packed_cpu`, SuperGenius commit `e1f28d7`), and flags a **stale SuperGenius test** `Fp4UltraRecognizedButDecodeUnavailable` asserting the old `FORMAT_UNSUPPORTED` behavior.
- NEO-SWARM `neoswarm/phases/04-sgprocessing-integration/04-02-PLAN.md:19-31` describes FP4_ULTRA as **"wire + stub"** returning `ProcessingError{FORMAT_UNSUPPORTED}` because MNN's E2M1 kernel was 39 commits diverged on `MNN_Ultra` at that time (D-04/D-08).
These two are from different dates/branches and I could not reconcile them without reading the SuperGenius repo (not in scope this session). **Whichever is current, note that SGFP4 is a separate concern from FP4_ULTRA — resolving the E2M1 wiring does not address SGFP4.**

### Bonus finding — a stale, self-contradicting GNUS-NEO-SWARM test (static inference, not run)
`test/integration/test_sg_connectivity.cpp:63-72` (`BuildSchemaJsonFP4UltraFormatEmitsFP4Type`) asserts the schema JSON **contains** the lowercase literal `"fp4_ultra"` (`EXPECT_NE(find("fp4_ultra"), npos)`). But Phase 04-01 (`83a0e84`) deliberately changed the bridge so the type field is `"tensor"` and only the *format* field is `"FP4_ULTRA"` (uppercase). A case-sensitive `find("fp4_ultra")` now returns `npos`, so this `EXPECT_NE` should **fail**. It directly contradicts `test/integration/test_sgprocessing_pipeline.cpp:96` which asserts `find("fp4_ultra") == npos`. Looks like 04-01 updated one test and left the connectivity test stale. Worth a quick fix regardless of the SGFP4 decision. (Confidence: MEDIUM — inferred statically, not executed.)

---

## Q4 — Integration gaps (concrete, file-level)

The key architectural clarification that reshapes this list: **MNN's SGFP4 is a weight-compression format, consumed as a graph op (`OpType_SGFP4Dequant`) that loads compressed weights from an external `.mnn.weight`-style sidecar** (`CPUSGFP4Dequant::onResize:43-89`, `VulkanSGFP4Dequant` creator `:124-205`). It is **not** an input-tensor decode entry point like `dequant_fp4_packed_cpu`. So "consuming SGFP4" most likely means *running a model whose weights were exported to SGFP4*, not *feeding SGFP4 as an input tensor through the FP4_ULTRA slot*. That distinction determines which of the gaps below actually apply.

**If SGFP4 stays a model-weight concern (likely correct reading):**
1. **MNN submodule / build** must include `OpType_SGFP4Dequant` + `MNN_SUPPORT_TRANSFORMER_FUSE` (the op and its tests are gated on this macro). GNUS-NEO-SWARM links MNN; confirm the linked build has the op registered.
2. **Model export** to SGFP4 sidecars — per GCS ROADMAP this belongs to `gnus-poc` ("SGFP4 quantization export | gnus-poc | poc (Python)", `neoswarm/ROADMAP.md:161`). `tools/fp4/encode_sgfp4.py` is the reference encoder to build on.
3. **`SGProcessingBridge` / `TensorInterpreter` need little-to-no change** — output is still FLOAT32; the container lives with the model, not the input.
4. **`fp4_codec.{hpp,cpp}` + `test/core/test_fp4_codec.cpp` should be deleted** (orphaned NF4 dead code, self-flagged D-13).

**Additionally, if GCS/SuperGenius must speak SGFP4 as an input/wire format (only if that's a real requirement):**
5. New `InputFormat::SGFP4_V2` in SuperGenius `generated/InputFormat.hpp`.
6. New cases in `InputFormatToTypeString` / `InputFormatToFormatString` (`sg_processing_bridge.cpp:40-95`).
7. New SGFP4 branch in SGProcessingManager's `ProcessingManager::CheckProcessValidity` + `MNN_Tensor` processor (currently only FP4_ULTRA/E2M1, per `04-02-PLAN.md`).
8. The **verifiable-execution / attestation** story (arxiv §8: bit-exact cross-device replay, ternary integer-exact path) — currently has **no anchor anywhere** in the codebase family (`SGFP4-PIVOT-ANALYSIS.md:146`); would be new work in both MNN and SuperGenius if pursued.

**Ownership reconciliation gap (important):** GCS `neoswarm/ROADMAP.md:173` assigns *"SGFP4 GPU decode shaders (Vulkan/MoltenVK)"* to GNUS-NEO-SWARM — but MNN's sgfp4-pivot has now built exactly that inside MNN (`sgfp4_dequant.comp`). This roadmap line is now duplicative/obsolete and needs an explicit decision: does GNUS-NEO-SWARM consume MNN's SGFP4 Vulkan decode, or was that ownership row superseded?

**Housekeeping gaps:** stale `test_sg_connectivity.cpp` fp4_ultra assertion (Q3); stale SuperGenius `Fp4UltraRecognizedButDecodeUnavailable` test (cross-repo); MNN-side stale REQUIREMENTS checkboxes + missing `02-VERIFICATION.md` (Q1).

---

## Q5 — Workstream recommendation input

**Is a dedicated new workstream warranted? Conditional — and the condition is a scoping decision the report should surface, not pre-empt.**

- **If the goal is "run SGFP4-quantized models through MNN via the existing SGProcessing path":** this is **NOT** workstream-sized. It's a handful of quick tasks: (a) confirm/bump the linked MNN build to include `OpType_SGFP4Dequant`; (b) add SGFP4 export in `gnus-poc`; (c) one end-to-end test running an SGFP4-weighted model; (d) a cleanup task deleting the orphaned NF4 `fp4_codec` + fixing the two stale FP4 tests. Bridge/interpreter are essentially unchanged because decode is internal to MNN and output is FLOAT32.

- **If the goal is the full cross-repo story** (GCS/SuperGenius speaking SGFP4 as a first-class format **and/or** wiring the arxiv §8 verifiable-execution/attestation use case): **yes, a dedicated workstream is warranted** — it spans three repos (poc export, MNN submodule integration, SuperGenius InputFormat + processor + verification), has genuine sequencing/dependencies, and touches the reputation/consensus verification layer that is the paper's whole motivation.

**Rough phase breakdown if the full workstream is chosen** (non-binding sketch; per CONTEXT.md, do NOT scaffold it in this task — recommend as follow-up, sited as a top-level `GeniusCognitiveSystem/.planning/workstreams/` entry pointing at the GNUS-NEO-SWARM submodule the way `.planning/workstreams/neo/config.json` does):
- **P1 — Reconcile & clean up:** delete orphaned NF4 `fp4_codec` + its test; fix stale `test_sg_connectivity.cpp` and (cross-repo) SuperGenius stale test; resolve the ROADMAP ownership row (MNN owns SGFP4 decode vs. GNUS-NEO-SWARM shaders); pin MNN submodule to a commit that includes `OpType_SGFP4Dequant`.
- **P2 — SGFP4 model export + MNN graph consumption:** `gnus-poc` exports weights to SGFP4 sidecars (build on `encode_sgfp4.py`); end-to-end inference of an SGFP4-weighted model through MNN LLM / SGProcessing; output-parity check.
- **P3 (only if SGFP4-as-input is required):** SuperGenius `InputFormat::SGFP4_V2` + bridge mapping + `ProcessingManager`/`MNN_Tensor` validation branch.
- **P4 (optional, arxiv §8):** verifiable-execution/attestation — bit-exact replay + ternary integer-exact determinism class; net-new in both MNN and SuperGenius.

**Recommended framing for the report author:** lead with the three-formats clarification and the "SGFP4 is weight-compression, not an input format" distinction — most of the apparent "integration gap" dissolves or refocuses once that is stated. Then give the conditional yes/no above rather than an unconditional one.

---

## Evidence Index (file:line)

**MNN sgfp4-pivot (`W:\gnus\GeniusNetwork\thirdparty\MNN`):**
- Spec: `.planning/sgfp4-arxiv-v2.txt` (§3.2 affine dual-mode; §4.3 packing; §6.1/6.2 v2 framing/records; §8 verifiable execution)
- Workstream docs: `.planning/workstreams/sgfp4-pivot/{REQUIREMENTS.md, ROADMAP.md, STATE.md}`; `SGFP4-PIVOT-ANALYSIS.md`; `260825-backfill-sgfp4-pivot-phase2-completion/SUMMARY.md`; `phases/01…/deferred-items.md`; `phases/04…/04-VERIFICATION.md`
- CPU decode core: `include/MNN/SGFP4DequantUtils.hpp:105-135` (uniform resolver), `:188-222` (quadtree walk), `:232-244` (FP16 unpack), `:262-282` (dual-mode payload), `:321-456` (container decode)
- CPU op: `source/backend/cpu/CPUSGFP4Dequant.cpp:43-122`
- Vulkan: `source/backend/vulkan/buffer/execution/glsl/sgfp4_dequant.comp:114-180, 189-292`; `VulkanSGFP4Dequant.cpp:124-211`
- Encoder: `tools/fp4/encode_sgfp4.py` (subdivide/build_split_map/classify/veto/selftest/fixture-emit)
- Tests: `test/op/SGFP4DequantTest.cpp` (uniform_decode + mixed_decode suites); `test/op/SGFP4VulkanDequantTest.cpp`; `test/op/SGFP4DequantFixtures.h`
- E2M1 contrast: `include/MNN/FP4DequantUtils.hpp::dequant_fp4_packed_cpu`

**GNUS-NEO-SWARM (`W:\gnus\GeniusCognitiveSystem\GNUS-NEO-SWARM`):**
- NF4 codec (orphaned): `src/core/fp4/fp4_codec.hpp:8-13, 32-33`; `fp4_codec.cpp:53-56, 124, 156-159`; test `test/core/test_fp4_codec.cpp`
- Bridge: `src/core/sgprocessing/sg_processing_bridge.cpp:40-95` (format maps), `:308-380` (SubmitDirect)
- Interpreter: `src/core/sgprocessing/tensor_interpreter.cpp:44-60`
- Engine: `src/core/engine/mnn_inference_engine.cpp:257-288` (InferViaSGProcessing; FP4_ULTRA label at `:266-267`)
- Stale test: `test/integration/test_sg_connectivity.cpp:63-72` vs `test/integration/test_sgprocessing_pipeline.cpp:96`
- Phase 04 integration history: `.planning/workstreams/neoswarm/phases/04-sgprocessing-integration/04-0{1,2}-PLAN.md`; ROADMAP ownership rows `.planning/workstreams/neoswarm/ROADMAP.md:161,173`

**GCS top-level:** `.planning/SUBREPOS.md`; `.planning/intel/classifications/sgfp4-format*.json`; `.planning/workstreams/neo/config.json`

## Confidence & caveats
- HIGH: both codebases' source (read in full), MNN phase completion, the three-formats finding, fp4_codec being orphaned.
- MEDIUM: SuperGenius/SGProcessingManager internals (secondhand via MNN analysis + NEO-SWARM plan docs; the SuperGenius repo was not read this session); the FP4_ULTRA live-vs-stub ambiguity is unresolved; the stale `test_sg_connectivity.cpp` failure is a static inference (not executed).
- No builds or tests were run.
