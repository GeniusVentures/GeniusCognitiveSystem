---
title: scripts::prepare_datasets

---

# scripts::prepare_datasets



 [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| | **[load_niche_config](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#function-load_niche_config)**() |
| | **[extract_niche_samples](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#function-extract_niche_samples)**(niche_name niche_name, niche_config niche_config, target_niches_config target_niches_config) |
| | **[create_splits](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#function-create_splits)**(samples samples, niche_name niche_name) |
| | **[format_for_training](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#function-format_for_training)**(samples samples, niche_name niche_name) |
| | **[save_datasets](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#function-save_datasets)**(niche_name niche_name, splits splits) |
| | **[main](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#function-main)**() |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[PROJECT_ROOT](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#variable-project_root)**  |
| dict | **[NCHE_TOKENIZER_MODELS](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#variable-nche_tokenizer_models)**  |
| list | **[SELECTED_NICHES](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#variable-selected_niches)**  |
| float | **[VAL_SPLIT](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#variable-val_split)**  |
| float | **[TEST_SPLIT](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#variable-test_split)**  |
| int | **[RANDOM_SEED](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#variable-random_seed)**  |
| int | **[MAX_SAMPLES_PER_NICHE](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#variable-max_samples_per_niche)**  |
| | **[OUTPUT_DIR](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#variable-output_dir)**  |
| | **[exist_ok](/python-reference/Namespaces/d2/dcb/namespacescripts_1_1prepare__datasets/#variable-exist_ok)**  |

## Detailed Description




```
Prepare training datasets for GNUS.ai specialists
Creates clean train/val splits from source-based niches
```


## Functions Documentation

### function load_niche_config

```python
load_niche_config()
```




```
Load the source-based niche analysis```


### function extract_niche_samples

```python
extract_niche_samples(
    niche_name niche_name,
    niche_config niche_config,
    target_niches_config target_niches_config
)
```




```
Extract all samples for a specific niche from Common Pile
```


### function create_splits

```python
create_splits(
    samples samples,
    niche_name niche_name
)
```




```
Create train/val/test splits
```


### function format_for_training

```python
format_for_training(
    samples samples,
    niche_name niche_name
)
```




```
Format samples for Qwen3 instruction tuning using tokenizer.apply_chat_template().

Per FOUND-01: Uses the actual tokenizer's native chat template (Qwen3 format)
instead of hand-rolled <|im_start|> strings that cause Qwen2.5/Qwen3 mismatch.
```


### function save_datasets

```python
save_datasets(
    niche_name niche_name,
    splits splits
)
```




```
Save as Hugging Face datasets for easy loading
```


### function main

```python
main()
```



## Attributes Documentation

### variable PROJECT_ROOT

```python
PROJECT_ROOT =  Path(__file__).resolve().parent.parent.parent;
```


### variable NCHE_TOKENIZER_MODELS

```python
dict NCHE_TOKENIZER_MODELS =  {
    "medical":      "mlx-community/Qwen3-30B-A3B-Instruct-2507-bf16",
    "qa_technical": "mlx-community/Qwen3-30B-A3B-Instruct-2507-bf16",
    "code":         "mlx-community/Qwen3-Coder-30B-A3B-Instruct-bf16",
    "encyclopedic": "mlx-community/Qwen3-30B-A3B-Instruct-2507-bf16",
    "patents":      "mlx-community/Qwen3-30B-A3B-Instruct-2507-bf16",
};
```


### variable SELECTED_NICHES

```python
list SELECTED_NICHES =  ['medical', 'qa_technical', 'code', 'encyclopedic', 'patents'];
```


### variable VAL_SPLIT

```python
float VAL_SPLIT =  0.1;
```


### variable TEST_SPLIT

```python
float TEST_SPLIT =  0.05;
```


### variable RANDOM_SEED

```python
int RANDOM_SEED =  42;
```


### variable MAX_SAMPLES_PER_NICHE

```python
int MAX_SAMPLES_PER_NICHE =  10000;
```


### variable OUTPUT_DIR

```python
OUTPUT_DIR =  str(PROJECT_ROOT / 'data' / 'specialists');
```


### variable exist_ok

```python
exist_ok;
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700