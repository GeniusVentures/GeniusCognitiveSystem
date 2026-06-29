---
title: training::train_specialists

---

# training::train_specialists



 [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| str | **[prepare_dataset_for_mlx](/python-reference/Namespaces/dd/df2/namespacetraining_1_1train__specialists/#function-prepare_dataset_for_mlx)**(str niche_name) |
| SimpleNamespace | **[build_args_for_niche](/python-reference/Namespaces/dd/df2/namespacetraining_1_1train__specialists/#function-build_args_for_niche)**(str niche_name, str base_model, str data_dir, str adapter_path) |
| | **[train_specialist](/python-reference/Namespaces/dd/df2/namespacetraining_1_1train__specialists/#function-train_specialist)**(str niche_name) |
| | **[main](/python-reference/Namespaces/dd/df2/namespacetraining_1_1train__specialists/#function-main)**() |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[PROJECT_ROOT](/python-reference/Namespaces/dd/df2/namespacetraining_1_1train__specialists/#variable-project_root)**  |
| dict | **[SPECIALIST_BASE_MODELS](/python-reference/Namespaces/dd/df2/namespacetraining_1_1train__specialists/#variable-specialist_base_models)**  |
| | **[SPECIALISTS](/python-reference/Namespaces/dd/df2/namespacetraining_1_1train__specialists/#variable-specialists)**  |
| | **[DATA_DIR](/python-reference/Namespaces/dd/df2/namespacetraining_1_1train__specialists/#variable-data_dir)**  |
| | **[OUTPUT_DIR](/python-reference/Namespaces/dd/df2/namespacetraining_1_1train__specialists/#variable-output_dir)**  |
| | **[parents](/python-reference/Namespaces/dd/df2/namespacetraining_1_1train__specialists/#variable-parents)**  |
| | **[True](/python-reference/Namespaces/dd/df2/namespacetraining_1_1train__specialists/#variable-true)**  |
| | **[exist_ok](/python-reference/Namespaces/dd/df2/namespacetraining_1_1train__specialists/#variable-exist_ok)**  |
| dict | **[OVERRIDES](/python-reference/Namespaces/dd/df2/namespacetraining_1_1train__specialists/#variable-overrides)**  |

## Detailed Description




```
Train GNUS.ai specialist models using mlx-lm's internal LoRA trainer.

DEPRECATED: Use train_specialists_mlx.py instead. This script lacks skip-logic
fixes (FOUND-02) and does not write TRAINING_STATUS.json. It trains with Qwen3-7B
base models rather than the MLX community Qwen3-30B-A3B variants used by the
primary pipeline.
```


## Functions Documentation

### function prepare_dataset_for_mlx

```python
str prepare_dataset_for_mlx(
    str niche_name
)
```




```
Convert HF dataset (saved with save_to_disk) into MLX-LM JSONL format:
  <data_dir>_mlx/train.jsonl
  <data_dir>_mlx/valid.jsonl

Each line: {"text": "..."}  (mlx-lm LORA.md 'text' format)
```


### function build_args_for_niche

```python
SimpleNamespace build_args_for_niche(
    str niche_name,
    str base_model,
    str data_dir,
    str adapter_path
)
```




```
Build args namespace compatible with mlx_lora.train_model(),
starting from CONFIG_DEFAULTS and applying overrides + required fields.
```


### function train_specialist

```python
train_specialist(
    str niche_name
)
```


### function main

```python
main()
```



## Attributes Documentation

### variable PROJECT_ROOT

```python
PROJECT_ROOT =  Path(__file__).resolve().parent.parent;
```


### variable SPECIALIST_BASE_MODELS

```python
dict SPECIALIST_BASE_MODELS =  {
    "medical":        "Qwen/Qwen3-7B-Instruct",
    "qa_technical":   "Qwen/Qwen3-7B-Instruct",
    "code":           "Qwen/Qwen3-7B-Coder",
    "encyclopedic":   "Qwen/Qwen3-7B-Instruct",
    "patents":        "Qwen/Qwen3-7B-Instruct",
};
```


### variable SPECIALISTS

```python
SPECIALISTS =  list(SPECIALIST_BASE_MODELS.keys());
```


### variable DATA_DIR

```python
DATA_DIR =  str(PROJECT_ROOT / "data" / "specialists");
```


### variable OUTPUT_DIR

```python
OUTPUT_DIR =  str(PROJECT_ROOT / "models" / "specialists");
```


### variable parents

```python
parents;
```


### variable True

```python
True;
```


### variable exist_ok

```python
exist_ok;
```


### variable OVERRIDES

```python
dict OVERRIDES =  {
    "fine_tune_type": "lora",    # use LoRA/QLoRA
    "optimizer": "adamw",
    "batch_size": 4,
    "iters": 1000,
    "val_batches": 25,
    "learning_rate": 1e-5,
    "steps_per_report": 50,
    "steps_per_eval": 200,
    "save_every": 200,
    "num_layers": 16,            # number of layers to LoRA-ize
    "grad_checkpoint": True,
    "grad_accumulation_steps": 1,
    "mask_prompt": False,
    "report_to": None,
    "project_name": None,
    "seed": 42,
    "lora_parameters": {         # MUST match what linear_to_lora_layers expects
        "rank": 16,
        "dropout": 0.05,
        "scale": 20.0,
    },
};
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700