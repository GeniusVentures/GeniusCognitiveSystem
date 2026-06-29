---
title: eval::evaluator::SpecialistEvaluator

---

# eval::evaluator::SpecialistEvaluator





## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d0/d3a/classeval_1_1evaluator_1_1_specialist_evaluator/#function-__init__)**(self self, Optional project_root[Path] =None) |
| dict | **[evaluate](/python-reference/Classes/d0/d3a/classeval_1_1evaluator_1_1_specialist_evaluator/#function-evaluate)**(self self, model model, tokenizer tokenizer, list test_samples, str niche_name) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| Optional[dict] | **[_evaluate_sample](/python-reference/Classes/d0/d3a/classeval_1_1evaluator_1_1_specialist_evaluator/#function-_evaluate_sample)**(self self, model model, tokenizer tokenizer, str text) |
| | **[_forward](/python-reference/Classes/d0/d3a/classeval_1_1evaluator_1_1_specialist_evaluator/#function-_forward)**(self self, model model, tokens tokens) |
| | **[_cross_entropy](/python-reference/Classes/d0/d3a/classeval_1_1evaluator_1_1_specialist_evaluator/#function-_cross_entropy)**(self self, logits logits, targets targets) |
| | **[_greedy_decode](/python-reference/Classes/d0/d3a/classeval_1_1evaluator_1_1_specialist_evaluator/#function-_greedy_decode)**(self self, model model, tokens tokens, max_new max_new) |
| float | **[_rouge_l](/python-reference/Classes/d0/d3a/classeval_1_1evaluator_1_1_specialist_evaluator/#function-_rouge_l)**(self self, str reference, str candidate) |
| int | **[_lcs_length](/python-reference/Classes/d0/d3a/classeval_1_1evaluator_1_1_specialist_evaluator/#function-_lcs_length)**(self self, list a, list b) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_project_root](/python-reference/Classes/d0/d3a/classeval_1_1evaluator_1_1_specialist_evaluator/#variable-_project_root)**  |

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    Optional project_root[Path] =None
)
```


### function evaluate

```python
dict evaluate(
    self self,
    model model,
    tokenizer tokenizer,
    list test_samples,
    str niche_name
)
```


## Protected Functions Documentation

### function _evaluate_sample

```python
Optional[dict] _evaluate_sample(
    self self,
    model model,
    tokenizer tokenizer,
    str text
)
```


### function _forward

```python
_forward(
    self self,
    model model,
    tokens tokens
)
```


### function _cross_entropy

```python
_cross_entropy(
    self self,
    logits logits,
    targets targets
)
```


### function _greedy_decode

```python
_greedy_decode(
    self self,
    model model,
    tokens tokens,
    max_new max_new
)
```


### function _rouge_l

```python
float _rouge_l(
    self self,
    str reference,
    str candidate
)
```


### function _lcs_length

```python
int _lcs_length(
    self self,
    list a,
    list b
)
```


## Protected Attributes Documentation

### variable _project_root

```python
_project_root =  project_root;
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700