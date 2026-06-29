---
title: training::train_specialists_mlx

---

# training::train_specialists_mlx



 [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| str | **[prepare_dataset_for_mlx](/python-reference/Namespaces/d7/df6/namespacetraining_1_1train__specialists__mlx/#function-prepare_dataset_for_mlx)**(str niche_name) |
| SimpleNamespace | **[build_args_for_niche](/python-reference/Namespaces/d7/df6/namespacetraining_1_1train__specialists__mlx/#function-build_args_for_niche)**(str niche_name, str base_model, str data_dir, str adapter_path) |
| | **[train_specialist](/python-reference/Namespaces/d7/df6/namespacetraining_1_1train__specialists__mlx/#function-train_specialist)**(str niche_name) |
| | **[main](/python-reference/Namespaces/d7/df6/namespacetraining_1_1train__specialists__mlx/#function-main)**() |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[PROJECT_ROOT](/python-reference/Namespaces/d7/df6/namespacetraining_1_1train__specialists__mlx/#variable-project_root)**  |
| dict | **[SPECIALIST_BASE_MODELS](/python-reference/Namespaces/d7/df6/namespacetraining_1_1train__specialists__mlx/#variable-specialist_base_models)**  |
| | **[SPECIALISTS](/python-reference/Namespaces/d7/df6/namespacetraining_1_1train__specialists__mlx/#variable-specialists)**  |
| | **[DATA_DIR](/python-reference/Namespaces/d7/df6/namespacetraining_1_1train__specialists__mlx/#variable-data_dir)**  |
| | **[OUTPUT_DIR](/python-reference/Namespaces/d7/df6/namespacetraining_1_1train__specialists__mlx/#variable-output_dir)**  |
| | **[parents](/python-reference/Namespaces/d7/df6/namespacetraining_1_1train__specialists__mlx/#variable-parents)**  |
| | **[True](/python-reference/Namespaces/d7/df6/namespacetraining_1_1train__specialists__mlx/#variable-true)**  |
| | **[exist_ok](/python-reference/Namespaces/d7/df6/namespacetraining_1_1train__specialists__mlx/#variable-exist_ok)**  |
| dict | **[OVERRIDES](/python-reference/Namespaces/d7/df6/namespacetraining_1_1train__specialists__mlx/#variable-overrides)**  |

## Detailed Description




```
Train GNUS.ai specialist models using mlx-lm's internal LoRA trainer.

Specialists:
  - medical        -> mlx-community/Qwen3-30B-A3B-Instruct-2507-bf16
  - qa_technical   -> mlx-community/Qwen3-30B-A3B-Instruct-2507-bf16
  - code           -> mlx-community/Qwen3-Coder-30B-A3B-Instruct-bf16
  - encyclopedic   -> mlx-community/Qwen3-30B-A3B-Instruct-2507-bf16
  - patents        -> mlx-community/Qwen3-30B-A3B-Instruct-2507-bf16

Data:
  - data/specialists/<niche> (HF datasets saved with save_to_disk)
  - This script converts each to:
      data/specialists/<niche>_mlx/{train,valid}.jsonl
    with {"text": "..."} lines as mlx-lm docs specify.

Pipeline (per specialist):
  - Build args from mlx_lm.lora.CONFIG_DEFAULTS + overrides
  - mlx_lm.utils.load(model_id) -> model, tokenizer
  - mlx_lm.tuner.datasets.load_dataset(args, tokenizer) -> train/val/test
  - mlx_lm.lora.train_model(args, model, train_set, valid_set)
```


## Functions Documentation

### function prepare_dataset_for_mlx

```python
str prepare_dataset_for_mlx(
    str niche_name
)
```




```
Convert HF dataset (save_to_disk) into MLX-LM JSONL format:
  data/specialists/<niche>_mlx/{train,valid}.jsonl

Each line: {"text": "..."}  (mlx-lm LORA.md 'text' format).
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
Build args namespace exactly like mlx_lm.lora.run() would,
but we call train_model() directly instead of run().
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
    "medical":      "mlx-community/Qwen3-30B-A3B-Instruct-2507-bf16",
    "qa_technical": "mlx-community/Qwen3-30B-A3B-Instruct-2507-bf16",
    "code":         "mlx-community/Qwen3-Coder-30B-A3B-Instruct-bf16",
    "encyclopedic": "mlx-community/Qwen3-30B-A3B-Instruct-2507-bf16",
    "patents":      "mlx-community/Qwen3-30B-A3B-Instruct-2507-bf16",
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
OUTPUT_DIR =  str(PROJECT_ROOT / "models" / "specialists_mlx");
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
    "fine_tune_type": "lora",    # LoRA/QLoRA
    "optimizer": "adamw",
    "batch_size": 4,
    "iters": 1000,               # drop to 200–400 while testing if needed
    "val_batches": 25,
    "learning_rate": 1e-5,
    "steps_per_report": 50,
    "steps_per_eval": 200,
    "save_every": 200,
    "num_layers": 16,            # how many layers to LoRA-ize (see docs)
    "grad_checkpoint": True,
    "grad_accumulation_steps": 1,
    "mask_prompt": False,
    "report_to": None,
    "project_name": None,
    "seed": 42,
    "lora_parameters": {
        "rank": 16,
        "dropout": 0.05,
        "scale": 20.0,
    },
};
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700