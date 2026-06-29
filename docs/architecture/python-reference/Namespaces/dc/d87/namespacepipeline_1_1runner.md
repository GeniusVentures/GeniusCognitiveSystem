---
title: pipeline::runner

---

# pipeline::runner



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[pipeline::runner::StageResult](/python-reference/Classes/dd/d61/classpipeline_1_1runner_1_1_stage_result/)**  |
| class | **[pipeline::runner::PipelineRunner](/python-reference/Classes/d4/daf/classpipeline_1_1runner_1_1_pipeline_runner/)**  |

## Functions

|                | Name           |
| -------------- | -------------- |
| None | **[main](/python-reference/Namespaces/dc/d87/namespacepipeline_1_1runner/#function-main)**() |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[logger](/python-reference/Namespaces/dc/d87/namespacepipeline_1_1runner/#variable-logger)**  |

## Detailed Description




```
Pipeline runner — sequential DAG with subprocess execution and validated checkpoints.

Executes the 7-stage training/distillation pipeline for each specialist niche
via subprocess, capturing stdout/stderr and checking exit codes. Integrates
with CheckpointValidator for per-stage output validation before marking a
stage complete.
```


## Functions Documentation

### function main

```python
None main()
```




```
Parse command-line arguments and run the pipeline.```



## Attributes Documentation

### variable logger

```python
logger =  logging.getLogger(__name__);
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700