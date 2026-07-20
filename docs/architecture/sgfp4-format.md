# 22 SGFP4 Adaptive Quantization Format

This section defines the **SGFP4** weight compression format used across GeniusCognitiveSystem. It replaces the earlier FP4 v3 codec with an adaptive mixed-bit scheme designed for GPU-friendly decode, consistent cross-device fidelity, and minimal per-block metadata overhead.

---

## 22.1 Design Goals

- **Runs everywhere:** Vulkan, MoltenVK, and MNN-compatible decode kernels suitable for low-end devices through high-end GPUs.
- **Consistent answers:** Devices differ in throughput and caching, not in model precision tiers.
- **Fast decode:** Per-workgroup branching with vectorized bit-unpack; fixed per-block payloads avoid variable-length scatter.
- **Simple paging:** All macroblocks use the same 2048-byte payload, enabling uniform memory addressing.

---

## 22.2 Macroblocks (Tiling)

Weight tensors of shape `[O, I]` are partitioned into **64x64 macroblocks**:

- `tiles_y = ceil(O / 64)`
- `tiles_x = ceil(I / 64)`
- `B = tiles_y * tiles_x` total blocks
- The tensor is zero-padded to a multiple of 64 in both dimensions before partitioning.

Each block `b` maps to row-major grid coordinates `(by, bx)` within the padded tensor.

---

## 22.3 Container Layout

A quantized tensor is stored as three parallel arrays plus shape metadata:

| Array | Type | Size | Purpose |
|-------|------|------|---------|
| `headers[B]` | `uint32` | `B` entries | Packed half2 scale + bias per block |
| `offsets[B]` | `uint32` | `B` entries | Byte offset into codes blob, with low 4 bits as flags |
| `codes_blob[]` | `bytes` | `B * 2048` bytes | Concatenated fixed-size per-block payloads |
| Shape | — | metadata | Original `(O, I)` tensor dimensions |

### 22.3.1 Alignment and Flags-in-Offsets

Each 2048-byte payload MUST be 16-byte aligned, guaranteeing `codesOffsetBytes % 16 == 0`. The low 4 bits of the stored offset are thus free for per-block mode flags:

- `offsets[b] = (codesOffsetBytes & ~0xF) | flags4`

On decode:

- `flags4 = offsets[b] & 0xF`
- `baseBytes = offsets[b] & ~0xF`

This embeds mode selection at zero additional memory cost.

---

## 22.4 Header (Scale + Bias Affine Decode)

Every macroblock uses a unified affine decode:

```
w_hat = S * code + Bias
```

The header is a single `uint32` packing two IEEE-754 FP16 values:

- **High 16 bits:** `scale_fp16` (S) — non-negative per-block scale
- **Low 16 bits:** `bias_fp16` (Bias) — learned/fit per-block offset

Decoded on GPU via `unpackHalf2x16(headers[b])` or equivalent.

---

## 22.5 Per-Block Mode Flags

`flags4` (low 4 bits of `offsets[b]`):

| Bit | Mask | Name | Description |
|-----|------|------|-------------|
| 0 | `0x1` | **MODE** | `0` = FP4_AFFINE, `1` = T158_AFFINE |
| 1 | `0x2` | ERROR_HINT | `0` = L2-selected, `1` = Pyramid-selected |
| 2 | `0x4` | reserved | — |
| 3 | `0x8` | reserved | — |

Only **bit 0** (MODE) is required for decode.

---

## 22.6 Quantization Modes

Both modes produce a fixed **2048-byte payload** (512 `uint32` words) per block.

### 22.6.1 FP4_AFFINE (MODE = 0)

4-bit signed codes `q ∈ [-8, 7]` (two's complement nibbles):

- **Packing:** 8 codes per `uint32`, 4 bits each. Total: 4096 codes × 4 bits = **2048 bytes**.
- **Decode:** `w_hat = S * q + Bias`

### 22.6.2 T158_AFFINE (MODE = 1)

Ternary codes `t ∈ {-1, 0, +1}` stored as 2-bit symbols:

| Bits | Value |
|------|-------|
| `00` | 0 |
| `01` | +1 |
| `10` | -1 |
| `11` | reserved (decode as 0) |

- **Packing:** 16 codes per `uint32`, 2 bits each. Active data in `uint32[0..255]` (1024 bytes); `uint32[256..511]` is zero-filled to preserve fixed payload size.
- **Decode:** `w_hat = S * t + Bias`

The ternary codebook plus affine scale/bias keeps this in the **~1.58-bit class** while preserving GPU decode simplicity.

---

## 22.7 Adaptive Mode Selection (Encoding)

The encoder evaluates both modes per block and chooses the better one:

1. **FP4_AFFINE candidate:** Scale search (e.g. 32-step logspace around initial guess `S0 = abs_max / 7.0`) with bias initialized to `mean(w)`. Codes are rounded and clipped to `[-8, 7]`.
2. **T158_AFFINE candidate:** Bias set to median or mean; scale derived from mean absolute deviation of centered values. Ternary codes assigned by thresholding.
3. **Selection rule:** Prefer ternary when its error is within a small tolerance of FP4:

   ```
   if err_t158 <= (1.0 + delta) * err_fp4: choose T158
   else: choose FP4
   ```

   Typical `delta` range: 0.05 – 0.20.

4. **Error metric:** Default is L2 norm; optional pyramid-weighted error (Gaussian/Laplacian) preserves low-frequency structure.

---

## 22.8 GPU Decode Procedure

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

## 22.9 Cross-Referencing

- **Semantic Core quantization** is described in [03 Model and Router §5.1.2](./model-and-router.md).
- **FP4 design overview** is in [02 System Overview §4.1.1](./system-overview.md).
- **Performance targets** are in [07 Execution and Performance §10](./execution-and-performance.md).
