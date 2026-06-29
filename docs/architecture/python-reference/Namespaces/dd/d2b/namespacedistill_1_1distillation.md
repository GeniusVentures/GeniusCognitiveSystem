---
title: distill::distillation

---

# distill::distillation



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[distill::distillation::Distiller](/python-reference/Classes/d1/d3c/classdistill_1_1distillation_1_1_distiller/)**  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[parser](/python-reference/Namespaces/dd/d2b/namespacedistill_1_1distillation/#variable-parser)**  |
| | **[required](/python-reference/Namespaces/dd/d2b/namespacedistill_1_1distillation/#variable-required)**  |
| | **[True](/python-reference/Namespaces/dd/d2b/namespacedistill_1_1distillation/#variable-true)**  |
| | **[help](/python-reference/Namespaces/dd/d2b/namespacedistill_1_1distillation/#variable-help)**  |
| | **[args](/python-reference/Namespaces/dd/d2b/namespacedistill_1_1distillation/#variable-args)**  |
| | **[project_root](/python-reference/Namespaces/dd/d2b/namespacedistill_1_1distillation/#variable-project_root)**  |
| | **[distiller](/python-reference/Namespaces/dd/d2b/namespacedistill_1_1distillation/#variable-distiller)**  |
| dict | **[loss_log](/python-reference/Namespaces/dd/d2b/namespacedistill_1_1distillation/#variable-loss_log)**  |
| str | **[out_dir](/python-reference/Namespaces/dd/d2b/namespacedistill_1_1distillation/#variable-out_dir)**  |
| | **[parents](/python-reference/Namespaces/dd/d2b/namespacedistill_1_1distillation/#variable-parents)**  |
| | **[exist_ok](/python-reference/Namespaces/dd/d2b/namespacedistill_1_1distillation/#variable-exist_ok)**  |
| | **[f](/python-reference/Namespaces/dd/d2b/namespacedistill_1_1distillation/#variable-f)**  |
| | **[indent](/python-reference/Namespaces/dd/d2b/namespacedistill_1_1distillation/#variable-indent)**  |

## Detailed Description




```
Logit-based knowledge distillation from teacher to student.```



## Attributes Documentation

### variable parser

```python
parser =  argparse.ArgumentParser(description="Run knowledge distillation for a specialist");
```


### variable required

```python
required;
```


### variable True

```python
True;
```


### variable help

```python
help;
```


### variable args

```python
args =  parser.parse_args();
```


### variable project_root

```python
project_root =  Path(__file__).resolve().parent.parent;
```


### variable distiller

```python
distiller =  Distiller();
```


### variable loss_log

```python
dict loss_log =  {
        "niche": args.niche,
        "losses": [float("inf")],
        "note": "Placeholder — run with model and tokenizer for real KD loss",
    };
```


### variable out_dir

```python
str out_dir =  project_root / "artifacts" / "distill";
```


### variable parents

```python
parents;
```


### variable exist_ok

```python
exist_ok;
```


### variable f

```python
f;
```


### variable indent

```python
indent;
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700