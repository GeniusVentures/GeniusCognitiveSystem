---
title: training::config::TrainingConfig

---

# training::config::TrainingConfig





## Public Functions

|                | Name           |
| -------------- | -------------- |
| dict | **[to_lora_params](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#function-to_lora_params)**(self self) |
| dict | **[to_args_dict](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#function-to_args_dict)**(self self) |
| "TrainingConfig" | **[from_yaml](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#function-from_yaml)**(cls cls, Path yaml_path, Optional specialist[str] =None) |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| str | **[fine_tune_type](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-fine_tune_type)**  |
| str | **[optimizer](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-optimizer)**  |
| int | **[batch_size](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-batch_size)**  |
| int | **[iters](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-iters)**  |
| int | **[val_batches](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-val_batches)**  |
| float | **[learning_rate](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-learning_rate)**  |
| int | **[steps_per_report](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-steps_per_report)**  |
| int | **[steps_per_eval](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-steps_per_eval)**  |
| int | **[save_every](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-save_every)**  |
| int | **[num_layers](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-num_layers)**  |
| bool | **[grad_checkpoint](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-grad_checkpoint)**  |
| int | **[grad_accumulation_steps](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-grad_accumulation_steps)**  |
| bool | **[mask_prompt](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-mask_prompt)**  |
| Optional | **[report_to](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-report_to)**  |
| Optional | **[project_name](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-project_name)**  |
| int | **[seed](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-seed)**  |
| int | **[lora_rank](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-lora_rank)**  |
| float | **[lora_dropout](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-lora_dropout)**  |
| float | **[lora_scale](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-lora_scale)**  |
| bool | **[use_qlora](/python-reference/Classes/d5/dc4/classtraining_1_1config_1_1_training_config/#variable-use_qlora)**  |

## Public Functions Documentation

### function to_lora_params

```python
dict to_lora_params(
    self self
)
```


### function to_args_dict

```python
dict to_args_dict(
    self self
)
```


### function from_yaml

```python
"TrainingConfig" from_yaml(
    cls cls,
    Path yaml_path,
    Optional specialist[str] =None
)
```


## Public Attributes Documentation

### variable fine_tune_type

```python
static str fine_tune_type =  "lora";
```


### variable optimizer

```python
static str optimizer =  "adamw";
```


### variable batch_size

```python
static int batch_size =  4;
```


### variable iters

```python
static int iters =  1000;
```


### variable val_batches

```python
static int val_batches =  25;
```


### variable learning_rate

```python
static float learning_rate =  1e-5;
```


### variable steps_per_report

```python
static int steps_per_report =  50;
```


### variable steps_per_eval

```python
static int steps_per_eval =  200;
```


### variable save_every

```python
static int save_every =  200;
```


### variable num_layers

```python
static int num_layers =  16;
```


### variable grad_checkpoint

```python
static bool grad_checkpoint =  True;
```


### variable grad_accumulation_steps

```python
static int grad_accumulation_steps =  1;
```


### variable mask_prompt

```python
static bool mask_prompt =  False;
```


### variable report_to

```python
static Optional report_to =  None;
```


### variable project_name

```python
static Optional project_name =  None;
```


### variable seed

```python
static int seed =  42;
```


### variable lora_rank

```python
static int lora_rank =  16;
```


### variable lora_dropout

```python
static float lora_dropout =  0.05;
```


### variable lora_scale

```python
static float lora_scale =  20.0;
```


### variable use_qlora

```python
static bool use_qlora =  True;
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700