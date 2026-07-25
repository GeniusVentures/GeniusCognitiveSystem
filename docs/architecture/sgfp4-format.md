# 22 SGFP4 Adaptive Quantization Format

**Specification status:** Normative  
**Implementation status:** In progress

This section defines the **SGFP4** weight compression format used across GeniusCognitiveSystem. It replaces the earlier FP4 v3 codec with a dual-profile mixed-bit scheme designed for GPU-friendly decode, consistent cross-device fidelity, minimal per-block metadata overhead, and bit-exact replay for execution attestation.

SGFP4 has two profiles:

- **Profile v1 (fixed-payload):** the original layout. Every 64x64 macroblock carries a fixed 2048-byte payload. Uniform addressing; simplest possible decode. Files use the `.fp4` extension.
- **Profile v2 (quadtree-adaptive):** a self-framed stream (`'SGF4'` magic, version `0x02`) in which each macroblock is subdivided by a quadtree into variable-sized leaves (64x64 down to 4x4) chosen by Laplacian-weighted error. Files use the `.sgfp4` extension.

Both profiles share the same code modes (FP4_AFFINE, T158_AFFINE), the same affine reconstruction, and the same normative code packing. Decode semantics for both profiles are closed and normative: independent implementations must produce bit-identical tensors from the same container.

The SGFP4 paper is the normative specification. Implementation work is in progress on [`GNUS-NEO-SWARM/develop`](https://github.com/GeniusVentures/GNUS-NEO-SWARM/tree/develop/gnus-poc/quantize), including `fp4_exporter.py`, `quadtree.py`, `sgfp4_format.py`, and the independent reference decoder `sgfp4_decoder.py`.

---

## 22.1 Design Goals

- **Runs everywhere:** Vulkan, MoltenVK, and MNN-compatible decode kernels suitable for low-end devices through high-end GPUs.
- **Consistent answers:** Devices differ in throughput and caching, not in model precision tiers.
- **Fast decode:** Per-workgroup branching with vectorized bit-unpack; v1 uses fixed per-block payloads to avoid variable-length scatter.
- **Simple paging (v1):** All macroblocks use the same 2048-byte payload, enabling uniform memory addressing.
- **Adaptive bitrate (v2):** Smooth regions of a tensor are stored as large leaves at ~1.58–4 bits/weight while high-error regions split toward 4x4 leaves, spending bits only where the error surface demands them.
- **Verifiable replay:** Decode semantics are fully specified so that the Execution Integrity System (EIS) can treat container decode as a Class A determinism target (see [29 Execution Integrity System §29.4.2](./execution-integrity-system.md)).

---

## 22.2 Macroblocks (Tiling)

Weight tensors of shape `[O, I]` are partitioned into **64x64 macroblocks** in both profiles:

- `tiles_y = ceil(O / 64)`
- `tiles_x = ceil(I / 64)`
- `B = tiles_y * tiles_x` total blocks
- The tensor is zero-padded to a multiple of 64 in both dimensions before partitioning.

Each block `b` maps to row-major grid coordinates `(by, bx)` within the padded tensor. In v2, each macroblock is further subdivided into square **leaves** of size 64, 32, 16, 8, or 4 by the quadtree encoder (§22.8).

---

## 22.3 v1 Profile — Container Layout

A v1-quantized tensor is stored as three parallel arrays plus shape metadata:

| Array | Type | Size | Purpose |
|-------|------|------|---------|
| `headers[B]` | `uint32` | `B` entries | Packed half2 scale + bias per block |
| `offsets[B]` | `uint32` | `B` entries | Byte offset into codes blob, with low 4 bits as flags |
| `codes_blob[]` | `bytes` | `B * 2048` bytes | Concatenated fixed-size per-block payloads |
| Shape | — | metadata | Original `(O, I)` tensor dimensions, carried by the model manifest |

### 22.3.1 Alignment and Flags-in-Offsets

Each 2048-byte payload MUST be 16-byte aligned, guaranteeing `codesOffsetBytes % 16 == 0`. The low 4 bits of the stored offset are thus free for per-block mode flags:

- `offsets[b] = (codesOffsetBytes & ~0xF) | flags4`

On decode:

- `flags4 = offsets[b] & 0xF`
- `baseBytes = offsets[b] & ~0xF`

This embeds mode selection at zero additional memory cost.

---

## 22.4 Affine Parameters (Scale + Bias)

Every block (v1) or leaf (v2) uses a unified affine decode:

```
w_hat = S * code + Bias
```

The parameters are a single `uint32` packing two IEEE-754 FP16 values:

- **High 16 bits:** `scale_fp16` (S) — non-negative per-block scale
- **Low 16 bits:** `bias_fp16` (Bias) — fit per-block offset

Decoded on GPU via `unpackHalf2x16(header)` or equivalent. Values are clipped to the FP16 range (±65504) at encode time.

**v2 note:** in the v2 profile, the low 4 bits of this packed word carry the leaf's mode flags (Eq. 22.1 below), so the decoder recovers the bias as `half(header & 0xFFF0)` — the bias is quantized to 12-bit mantissa precision:

```
leaf_header = packHalf2x16(S, Bias) with low 4 bits replaced by flags4   (22.1)
```

---

## 22.5 Per-Block Mode Flags

**v1** — `flags4` (low 4 bits of `offsets[b]`):

| Bit | Mask | Name | Description |
|-----|------|------|-------------|
| 0 | `0x1` | **MODE** | `0` = FP4_AFFINE, `1` = T158_AFFINE |
| 1 | `0x2` | ERROR_HINT | `0` = L2-selected, `1` = Pyramid-selected |
| 2 | `0x4` | reserved | written 0 |
| 3 | `0x8` | reserved | written 0 |

**v2** — `flags4` (low 4 bits of each leaf header, per Eq. 22.1):

| Bit | Mask | Name | Description |
|-----|------|------|-------------|
| 0 | `0x1` | **MODE** | `0` = FP4_AFFINE, `1` = T158_AFFINE |
| 1 | `0x2` | reserved | written 0 (log mode, reserved) |
| 2 | `0x4` | reserved | written 0 |
| 3 | `0x8` | reserved | written 0 |

Only **bit 0** (MODE) is required for decode.

---

## 22.6 Quantization Modes and Payload Packing

Both modes are available in both profiles. Payload size scales with leaf area:

| Leaf size | Weights | FP4 payload (u32 words) | T158 payload (u32 words) |
|-----------|---------|--------------------------|---------------------------|
| 64x64 (v1 fixed) | 4096 | 512 (2048 B) | 256 active + 256 zero (2048 B) |
| 32x32 | 1024 | 128 | 64 |
| 16x16 | 256 | 32 | 16 |
| 8x8 | 64 | 8 | 4 |
| 4x4 | 16 | 2 | 1 |

### 22.6.1 FP4_AFFINE (MODE = 0)

4-bit signed codes `q ∈ [-8, 7]` (two's complement nibbles):

- **Packing:** 8 codes per `uint32`, 4 bits each. Code `i` occupies bits `4*(i mod 8)` of word `floor(i/8)`. Within each word, codes are packed LSB-first in row-major weight order.
- **Decode:** `w_hat = S * q + Bias`

### 22.6.2 T158_AFFINE (MODE = 1)

Ternary codes `t ∈ {-1, 0, +1}` stored as 2-bit symbols:

| Bits | Value |
|------|-------|
| `00` | 0 |
| `01` | +1 |
| `10` | -1 |
| `11` | reserved (decode as 0) |

- **Packing:** 16 codes per `uint32`, 2 bits each. Code `i` occupies bits `2*(i mod 16)` of word `floor(i/16)`. In the v1 64x64 payload, active data occupies `uint32[0..255]` (1024 bytes); `uint32[256..511]` MUST be written as zero to preserve the fixed payload size.
- **Decode:** `w_hat = S * t + Bias`

The ternary codebook plus affine scale/bias keeps this in the **~1.58-bit class** while preserving GPU decode simplicity. The T158 integer decode path (shift/mask + table lookup + integer multiply-add) is the preferred target for EIS Class A reference-integer semantics.

---

## 22.7 Encoding (Non-Normative)

The container formats are encoder-agnostic; any encoder producing conformant bytes is valid. The reference encoder proceeds as follows.

**Per-leaf candidate fitting:**

1. **FP4_AFFINE candidate:** bias initialized to `mean(w)`; scale seeded at `S0 = abs_max / 7.0` and refined by a 16-candidate log-space search over `[0.5, 1.5] × S0`, minimizing L2 error. Codes are rounded and clipped to `[-8, 7]`.
2. **T158_AFFINE candidate:** bias set to `mean(w)`; scale derived from mean absolute deviation of centered values. Ternary codes assigned by thresholding at `tau = S/2`.

**Dual-mode selection (per leaf):** reconstruct both candidates with their *own* codebooks and compare Laplacian-weighted error:

```
if err_t158 <= (1.0 + delta) * err_fp4: prefer T158
else: choose FP4
```

Default `delta = 0.10` (typical range 0.05–0.20). The Laplacian-weighted error emphasizes low-frequency structure of the weight tile over isolated high-frequency error. A per-weight outlier guard rejects T158 if any single weight's reconstruction error exceeds `5 * S`, forcing FP4.

**v2 quadtree split policy:** the encoder attempts the largest block first and splits a block into four children when its selected-mode error exceeds the size-dependent threshold. Hysteresis stabilizes the tree: a split of an already-accepted parent requires 20% error improvement, and a borderline block within 10% of threshold is accepted without splitting. Recursion bottoms out at 4x4 leaves.

---

## 22.8 v2 Profile — Quadtree-Adaptive Stream

### 22.8.1 File Framing

```
magic[4] 'SGF4' | version[1] = 0x02 | num_superblocks B (uint32 LE) |
pad[7] zero | record_offsets[B] (uint32 LE) | records[0..B-1]
```

- The 7-byte pad aligns the header to 16 bytes; the record region begins at byte `16 + 4*B`.
- `record_offsets[b]` is relative to the record region and MUST be a multiple of 16. Each record is zero-padded to a 16-byte multiple, so record alignment is preserved across the stream.

### 22.8.2 Record Layout

Each macroblock record is:

```
sb_header[4] | split_map[12] (LAYOUT_MIXED only) | block_headers[N*4] |
pad[0-15] zero | payloads[N] (each 16-byte padded)
```

`sb_header` is a `uint32`: bits 0–2 hold the layout enumeration, bits 3–31 are reserved (written 0):

| Layout | Name | Leaves | Leaf size | Split map |
|--------|------|--------|-----------|-----------|
| 0 | UNIFORM_64 | 1 | 64x64 | absent |
| 1 | UNIFORM_32 | 4 | 32x32 | absent |
| 2 | UNIFORM_16 | 16 | 16x16 | absent |
| 3 | UNIFORM_8 | 64 | 8x8 | absent |
| 4 | MIXED | variable | variable | **present (12 bytes)** |
| 5 | FULL_4x4 | 256 | 4x4 | absent |

**Leaf storage order:** uniform layouts store leaves in **row-major raster order** of the tile grid — the leaf geometry is fully implied by the layout value, so no split map is stored. LAYOUT_MIXED stores leaves in **pre-order DFS order** matching the split map.

**Split map (MIXED only):** 12 bytes = three little-endian `uint32` words, 85 bits maximum. Visiting nodes in pre-order DFS with quadrant order TL, TR, BL, BR, one bit per node of size ≥ 8 records whether the node is split (1) or a leaf (0). 4x4 nodes cannot split and contribute no bit. Bit `k` of the map is bit `k mod 32` of word `floor(k/32)`; unused upper bits are zero. The split map plus the fixed recursion floor lets a decoder rebuild the exact leaf geometry `(y, x, size)` with no other side information.

**Block headers:** N packed half2 words as in §22.4, with low 4 bits replaced by flags (Eq. 22.1).

**Payloads:** sizes per §22.6, each zero-padded to a 16-byte multiple. Because the record itself is 16-byte aligned and the header section is padded, intra-record alignment implies absolute-address alignment.

### 22.8.3 v2 Decode Procedure

1. Verify magic/version; read `B` and the offset table.
2. Per record: read layout. For uniform layouts, generate raster leaf positions; for MIXED, walk the split map to recover leaf positions in DFS order.
3. Per leaf: unpack `S` and `Bias` (bias via `header & 0xFFF0`), branch on mode bit 0, extract codes per §22.6, reconstruct `w = S * code + Bias`, and place the leaf at `(y, x)` within the 64x64 macroblock.
4. Assemble macroblocks in row-major grid order; crop the tensor to `(O, I)`.

---

## 22.9 GPU Decode Procedure (v1)

For each block `b`:

1. Unpack `S, Bias` from `headers[b]` via `unpackHalf2x16`.
2. Decode `flags4 = offsets[b] & 0xF`, `baseBytes = offsets[b] & ~0xF`.
3. Load 512 `uint32` words from `codes_blob[baseBytes]`.
4. Branch on `mode = flags4 & 0x1`:
   - **FP4:** Unpack nibbles → int4 codes → `w = S * code + Bias`.
   - **T158:** Unpack 2-bit symbols → map to {-1,0,+1} → `w = S * code + Bias`.
5. Write decoded 64x64 block to padded tensor; crop to `(O, I)`.

A single GPU workgroup decodes one macroblock, with each thread processing multiple weight values via vectorized bit unpacking.

---

## 22.10 Conformance and Bit-Exact Replay

Decode semantics in this chapter are **normative**: two independent implementations given the same container and shape metadata must produce bit-identical float32 tensors.

- The reference decoder (`gnus-poc/quantize/sgfp4_decoder.py`) implements only this chapter's semantics and is the conformance baseline. Encoder changes are validated by round-trip through the reference decoder plus hand-constructed golden vectors (including the reserved ternary symbol `11 → 0`).
- Container hashes pin the exact bytes in the EIS execution contract; bit-exact decode makes the *decoded weight tensor* a contract-grade artifact as well.
- The T158 integer decode path targets **EIS Class A (reference-integer semantics)**; FP4-affine paths with FP16 accumulation are expected to register as **Class B (bounded-drift)** with declared accumulation width and reduction order (see [§29.4](./execution-integrity-system.md)).

---

## 22.11 Cross-Referencing

- **Semantic Core quantization** is described in [03 Model and Router §5.1.2](./model-and-router.md).
- **SGFP4 design overview** is in [02 System Overview §4.1.1](./system-overview.md).
- **Execution-contract and determinism classes** are in [29 Execution Integrity System](./execution-integrity-system.md).
- **Performance targets** are in [07 Execution and Performance §10](./execution-and-performance.md).
