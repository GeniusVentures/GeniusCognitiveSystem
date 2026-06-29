---
title: pipeline::checkpoint

---

# pipeline::checkpoint



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[pipeline::checkpoint::StageValidationResult](/python-reference/Classes/d1/d15/classpipeline_1_1checkpoint_1_1_stage_validation_result/)**  |
| class | **[pipeline::checkpoint::CheckpointValidator](/python-reference/Classes/d9/db3/classpipeline_1_1checkpoint_1_1_checkpoint_validator/)**  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[logger](/python-reference/Namespaces/d5/d9f/namespacepipeline_1_1checkpoint/#variable-logger)**  |

## Detailed Description




```
Checkpoint validator — per-stage output validation for pipeline resume.

Replaces empty .done marker files with validated JSON checkpoints that verify
stage output quality before marking a stage complete.
```



## Attributes Documentation

### variable logger

```python
logger =  logging.getLogger(__name__);
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700