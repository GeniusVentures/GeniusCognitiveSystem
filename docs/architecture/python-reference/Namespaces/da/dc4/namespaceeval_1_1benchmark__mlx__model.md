---
title: eval::benchmark_mlx_model

---

# eval::benchmark_mlx_model



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[eval::benchmark_mlx_model::MLXBenchmarkModel](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/)**  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| int | **[kDefaultMaxLength](/python-reference/Namespaces/da/dc4/namespaceeval_1_1benchmark__mlx__model/#variable-kdefaultmaxlength)**  |
| int | **[kDefaultMaxGenToks](/python-reference/Namespaces/da/dc4/namespaceeval_1_1benchmark__mlx__model/#variable-kdefaultmaxgentoks)**  |

## Detailed Description




```
MLX model wrapper for lm-eval-harness LM interface.

Subclasses ``lm_eval.api.model.LM`` to enable in-process inference with
MLX quantized specialist models through lm-eval's standard evaluation protocol.

Per RESEARCH.md: model is loaded once in ``__init__`` and reused for all
benchmark tasks in a ``simple_evaluate()`` call (Pitfall 3 avoidance).

Threat mitigations:
- T-04-01: Paths validated with FileNotFoundError before any MLX import.
- T-04-04: max_gen_toks capped at model context window in generate_until().
```



## Attributes Documentation

### variable kDefaultMaxLength

```python
int kDefaultMaxLength =  2048;
```


### variable kDefaultMaxGenToks

```python
int kDefaultMaxGenToks =  256;
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700