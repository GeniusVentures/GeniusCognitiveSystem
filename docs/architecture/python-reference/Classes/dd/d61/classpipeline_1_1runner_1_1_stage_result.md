---
title: pipeline::runner::StageResult

---

# pipeline::runner::StageResult



 [More...](#detailed-description)

Inherits from NamedTuple

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| dict | **[_STAGE_COMMANDS](/python-reference/Classes/dd/d61/classpipeline_1_1runner_1_1_stage_result/#variable-_stage_commands)**  |

## Detailed Description

```python
class pipeline::runner::StageResult;
```




```
Outcome of a single pipeline stage execution.

Attributes:
    stage: Stage name (e.g., "train").
    niche: Specialist niche name (e.g., "code").
    success: Whether the stage completed successfully (exit 0).
    exit_code: Process exit code, or -1 if an exception occurred.
    stdout: Captured stdout from the subprocess.
    stderr: Captured stderr from the subprocess.
    attempts: Number of execution attempts (1 + retries).
```

## Protected Attributes Documentation

### variable _STAGE_COMMANDS

```python
static dict _STAGE_COMMANDS =  {
    "data_prep": ["data/scripts/prepare_datasets.py", "--niche", "{niche}"],
    "synthetic_data": ["distill/synthetic.py", "--niche", "{niche}"],
    "dedup": ["training/dedup.py", "--niche", "{niche}"],
    "train": ["training/train_specialists_mlx.py", "--niche", "{niche}"],
    "evaluate": ["eval/evaluator.py", "--niche", "{niche}"],
    "distill": ["distill/distillation.py", "--niche", "{niche}"],
    "quantize": ["quantize/fp4_exporter.py", "--niche", "{niche}"],
};
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700