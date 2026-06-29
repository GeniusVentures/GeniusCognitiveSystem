---
title: pipeline::runner::PipelineRunner

---

# pipeline::runner::PipelineRunner



 [More...](#detailed-description)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#function-__init__)**(self self, Optional project_root[Path] =None, Optional config_path[Path] =None) |
| None | **[run](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#function-run)**(self self, Optional niche[str] =None, Optional from_stage[str] =None, bool force =False) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| None | **[_load_config](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#function-_load_config)**(self self) |
| [StageResult](/python-reference/Classes/dd/d61/classpipeline_1_1runner_1_1_stage_result/) | **[_run_stage](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#function-_run_stage)**(self self, str niche, str stage) |
| List[str] | **[_build_command](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#function-_build_command)**(self self, str niche, str stage) |
| List[str] | **[_load_niches](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#function-_load_niches)**(self self) |
| int | **[_stage_index](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#function-_stage_index)**(self self, str stage_name) |
| bool | **[_is_complete](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#function-_is_complete)**(self self, str niche, str stage) |
| None | **[_mark_complete](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#function-_mark_complete)**(self self, str niche, str stage) |
| None | **[_print_success_output](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#function-_print_success_output)**(str stage, [StageResult](/python-reference/Classes/dd/d61/classpipeline_1_1runner_1_1_stage_result/) result) |
| None | **[_print_failure_output](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#function-_print_failure_output)**(str stage, [StageResult](/python-reference/Classes/dd/d61/classpipeline_1_1runner_1_1_stage_result/) result) |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| list | **[STAGES](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#variable-stages)**  |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| int | **[_kDefaultRetryCount](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#variable-_kdefaultretrycount)**  |
| float | **[_kDefaultBackoffSeconds](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#variable-_kdefaultbackoffseconds)**  |
| int | **[_kDefaultStageTimeout](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#variable-_kdefaultstagetimeout)**  |
| Path | **[_root](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#variable-_root)**  |
| Path | **[_config_path](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#variable-_config_path)**  |
| dict | **[_config](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#variable-_config)**  |
| int | **[_stage_retry_count](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#variable-_stage_retry_count)**  |
| float | **[_stage_backoff_seconds](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#variable-_stage_backoff_seconds)**  |
| | **[_checkpoint](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/#variable-_checkpoint)**  |

## Detailed Description

```python
class pipeline::runner::PipelineRunner;
```




```
Orchestrates the 7-stage pipeline for all specialist niches.

Loads configuration from YAML, executes each stage via subprocess with
stdout/stderr capture, validates outputs with CheckpointValidator, and
supports --force and --from-stage flags for checkpoint control.
```

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    Optional project_root[Path] =None,
    Optional config_path[Path] =None
)
```




```
Initialize the pipeline runner.

Args:
    project_root: Root directory of the gnus-poc project.
        Defaults to the parent of this file's directory.
    config_path: Path to ``pipeline.yaml``. Defaults to
        ``{project_root}/config/pipeline.yaml``.
```


### function run

```python
None run(
    self self,
    Optional niche[str] =None,
    Optional from_stage[str] =None,
    bool force =False
)
```




```
Run the pipeline for all niches (or a single niche).

Args:
    niche: Run for a single specialist niche. If ``None``, runs for
        all niches listed in ``pipeline.yaml``.
    from_stage: Stage name to resume from (inclusive). Earlier stages
        are skipped if their checkpoints exist.
    force: If ``True``, clear all checkpoints and re-run every stage.
```


## Protected Functions Documentation

### function _load_config

```python
None _load_config(
    self self
)
```




```
Load pipeline configuration from YAML file.```


### function _run_stage

```python
StageResult _run_stage(
    self self,
    str niche,
    str stage
)
```




```
Execute a single pipeline stage for the given niche via subprocess.

Handles retry, timeout, and per-D-10 error-type classification.
```


### function _build_command

```python
List[str] _build_command(
    self self,
    str niche,
    str stage
)
```




```
Build the subprocess command list for a given niche and stage.

Uses ``sys.executable`` for the Python interpreter so the same
environment is used for subprocess stages.
```


### function _load_niches

```python
List[str] _load_niches(
    self self
)
```




```
Load the list of specialist niches from configuration.```


### function _stage_index

```python
int _stage_index(
    self self,
    str stage_name
)
```




```
Return the zero-based index of *stage_name* in ``STAGES``.

Returns 0 if the name is not found (treat unknown as start).
```


### function _is_complete

```python
bool _is_complete(
    self self,
    str niche,
    str stage
)
```




```
Check whether a validated checkpoint exists for this niche/stage.```


### function _mark_complete

```python
None _mark_complete(
    self self,
    str niche,
    str stage
)
```




```
Validate stage outputs and write a checkpoint file if they pass.```


### function _print_success_output

```python
static None _print_success_output(
    str stage,
    StageResult result
)
```




```
Print a summary of successful stage output.```


### function _print_failure_output

```python
static None _print_failure_output(
    str stage,
    StageResult result
)
```




```
Print diagnostic information for a failed stage.```


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

### variable _kDefaultRetryCount

```python
static int _kDefaultRetryCount =  1;
```


### variable _kDefaultBackoffSeconds

```python
static float _kDefaultBackoffSeconds =  5.0;
```


### variable _kDefaultStageTimeout

```python
static int _kDefaultStageTimeout =  3600;
```


### variable _root

```python
Path _root =  project_root;
```


### variable _config_path

```python
Path _config_path =  config_path;
```


### variable _config

```python
dict _config =  {};
```


### variable _stage_retry_count

```python
int _stage_retry_count =  self._kDefaultRetryCount;
```


### variable _stage_backoff_seconds

```python
float _stage_backoff_seconds =  self._kDefaultBackoffSeconds;
```


### variable _checkpoint

```python
_checkpoint =  CheckpointValidator(self._root);
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700