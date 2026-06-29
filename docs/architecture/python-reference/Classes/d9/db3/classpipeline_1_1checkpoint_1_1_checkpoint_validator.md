---
title: pipeline::checkpoint::CheckpointValidator

---

# pipeline::checkpoint::CheckpointValidator



 [More...](#detailed-description)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-__init__)**(self self, Path project_root) |
| Path | **[checkpoint_dir](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-checkpoint_dir)**(self self) |
| Path | **[checkpoint_path](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-checkpoint_path)**(self self, str niche, str stage) |
| [StageValidationResult](/python-reference/Classes/d1/d15/classpipeline_1_1checkpoint_1_1_stage_validation_result/) | **[validate_stage](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-validate_stage)**(self self, str niche, str stage) |
| bool | **[is_complete](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-is_complete)**(self self, str niche, str stage) |
| None | **[mark_complete](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-mark_complete)**(self self, str niche, str stage, [StageValidationResult](/python-reference/Classes/d1/d15/classpipeline_1_1checkpoint_1_1_stage_validation_result/) result) |
| None | **[clear_checkpoint](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-clear_checkpoint)**(self self, str niche, str stage) |
| None | **[clear_all_checkpoints](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-clear_all_checkpoints)**(self self, str niche) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| List[Dict[str, Any]] | **[_validate_data_prep](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-_validate_data_prep)**(self self, str niche) |
| List[Dict[str, Any]] | **[_validate_synthetic_data](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-_validate_synthetic_data)**(self self, str niche) |
| List[Dict[str, Any]] | **[_validate_dedup](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-_validate_dedup)**(self self, str niche) |
| List[Dict[str, Any]] | **[_validate_train](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-_validate_train)**(self self, str niche) |
| List[Dict[str, Any]] | **[_validate_evaluate](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-_validate_evaluate)**(self self, str niche) |
| List[Dict[str, Any]] | **[_validate_distill](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-_validate_distill)**(self self, str niche) |
| List[Dict[str, Any]] | **[_validate_quantize](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-_validate_quantize)**(self self, str niche) |
| None | **[_check_sgfp4_magic_header](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-_check_sgfp4_magic_header)**(self self, List] checks[Dict[str, Any], Path sgfp4_path) |
| None | **[_check_manifest_sha256](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-_check_manifest_sha256)**(self self, List] checks[Dict[str, Any], Path manifest_path, Path sgfp4_path) |
| None | **[_check_manifest_required_fields](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-_check_manifest_required_fields)**(self self, List] checks[Dict[str, Any], Path manifest_path) |
| str | **[_file_sha256](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#function-_file_sha256)**(Path file_path) |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| list | **[STAGES](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#variable-stages)**  |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| int | **[_kMinSyntheticRowCount](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#variable-_kminsyntheticrowcount)**  |
| bytes | **[_kSgfp4Magic](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#variable-_ksgfp4magic)**  |
| int | **[_kSgfp4Version](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#variable-_ksgfp4version)**  |
| tuple | **[_kQuantManifestRequiredFields](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#variable-_kquantmanifestrequiredfields)**  |
| int | **[_kSha256ChunkSize](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#variable-_ksha256chunksize)**  |
| | **[_root](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/#variable-_root)**  |

## Detailed Description

```python
class pipeline::checkpoint::CheckpointValidator;
```




```
Validates pipeline stage outputs before marking a stage complete.

Each stage has specific validation checks (per D-15) that verify output
files exist, contain expected data, and meet quality thresholds.
```

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    Path project_root
)
```




```
Initialize the validator.

Args:
    project_root: Root directory of the gnus-poc project.
```


### function checkpoint_dir

```python
Path checkpoint_dir(
    self self
)
```




```
Directory where validated checkpoint JSON files are stored.```


### function checkpoint_path

```python
Path checkpoint_path(
    self self,
    str niche,
    str stage
)
```




```
Return the JSON checkpoint file path for a niche/stage pair.```


### function validate_stage

```python
StageValidationResult validate_stage(
    self self,
    str niche,
    str stage
)
```




```
Run all validation checks for a given niche and stage.

Args:
    niche: Specialist niche name (e.g., "code").
    stage: Pipeline stage name (e.g., "train").

Returns:
    StageValidationResult with per-check details.

Raises:
    ValueError: If *stage* is not one of the known pipeline stages.
```


### function is_complete

```python
bool is_complete(
    self self,
    str niche,
    str stage
)
```




```
Check if a validated checkpoint exists for the given niche/stage.

Returns ``True`` only when a JSON checkpoint file exists and its
``passed`` field is ``true``.
```


### function mark_complete

```python
None mark_complete(
    self self,
    str niche,
    str stage,
    StageValidationResult result
)
```




```
Write a validated checkpoint file to disk.

Sets *result.completed_at* to the current UTC timestamp and writes
the JSON to ``artifacts/.checkpoints/{niche}/{stage}.json``.

Raises:
    OSError: If the checkpoint cannot be written (disk full, etc.).
```


### function clear_checkpoint

```python
None clear_checkpoint(
    self self,
    str niche,
    str stage
)
```




```
Remove the checkpoint file for a single niche/stage, if it exists.```


### function clear_all_checkpoints

```python
None clear_all_checkpoints(
    self self,
    str niche
)
```




```
Remove all checkpoint files for a given niche (used by --force).```


## Protected Functions Documentation

### function _validate_data_prep

```python
List[Dict[str, Any]] _validate_data_prep(
    self self,
    str niche
)
```




```
Validate data_prep stage: dataset directory has non-init files.```


### function _validate_synthetic_data

```python
List[Dict[str, Any]] _validate_synthetic_data(
    self self,
    str niche
)
```




```
Validate synthetic_data stage: JSONL with minimum rows, valid JSON.```


### function _validate_dedup

```python
List[Dict[str, Any]] _validate_dedup(
    self self,
    str niche
)
```




```
Validate dedup stage: hash file and dedup log exist.```


### function _validate_train

```python
List[Dict[str, Any]] _validate_train(
    self self,
    str niche
)
```




```
Validate train stage: adapter weights, config, and metadata exist.```


### function _validate_evaluate

```python
List[Dict[str, Any]] _validate_evaluate(
    self self,
    str niche
)
```




```
Validate evaluate stage: evaluation JSON with required metrics.```


### function _validate_distill

```python
List[Dict[str, Any]] _validate_distill(
    self self,
    str niche
)
```




```
Validate distill stage: loss log exists with non-increasing trend.```


### function _validate_quantize

```python
List[Dict[str, Any]] _validate_quantize(
    self self,
    str niche
)
```




```
Validate quantize stage: FP4 export files, manifest, and SGFP4 v2 artifacts.

Checks:
1. fp4_dir_exists — the fp4 output directory exists
2. fp4_weights_exist — at least one .npz, .safetensors, or .sgfp4 file
3. manifest_exists — manifest.json exists
4. sgfp4_binary_exists — the {niche}.sgfp4 v2 binary exists (warning if missing)
5. magic_header_valid — .sgfp4 file starts with b'SGF4' + 0x02
6. manifest_sha256_valid — manifest fp4_binary.sha256 matches .sgfp4 file hash
7. manifest_required_fields — QUANT-03 required fields present in manifest
```


### function _check_sgfp4_magic_header

```python
None _check_sgfp4_magic_header(
    self self,
    List] checks[Dict[str, Any],
    Path sgfp4_path
)
```




```
Validate the SGFP4 v2 magic header (b'SGF4' + 0x02).```


### function _check_manifest_sha256

```python
None _check_manifest_sha256(
    self self,
    List] checks[Dict[str, Any],
    Path manifest_path,
    Path sgfp4_path
)
```




```
Verify manifest fp4_binary.sha256 matches the .sgfp4 file content hash.```


### function _check_manifest_required_fields

```python
None _check_manifest_required_fields(
    self self,
    List] checks[Dict[str, Any],
    Path manifest_path
)
```




```
Validate QUANT-03 required fields in manifest.json.```


### function _file_sha256

```python
static str _file_sha256(
    Path file_path
)
```




```
Compute the SHA256 hex digest of a file (streaming, 64 KiB chunks).```


## Public Attributes Documentation

### variable STAGES

```python
static list STAGES =  [
        "data_prep",
        "synthetic_data",
        "dedup",
        "train",
        "evaluate",
        "distill",
        "quantize",
    ];
```


## Protected Attributes Documentation

### variable _kMinSyntheticRowCount

```python
static int _kMinSyntheticRowCount =  10;
```


### variable _kSgfp4Magic

```python
static bytes _kSgfp4Magic =  b"SGF4";
```


### variable _kSgfp4Version

```python
static int _kSgfp4Version =  0x02;
```


### variable _kQuantManifestRequiredFields

```python
static tuple _kQuantManifestRequiredFields =  (
        "model_name",
        "niche",
        "base_model_ref",
        "adapter_ref",
        "quantization_params",
        "encoder_version",
        "timestamp_utc",
    );
```


### variable _kSha256ChunkSize

```python
static int _kSha256ChunkSize =  65536;
```


### variable _root

```python
_root =  project_root;
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700