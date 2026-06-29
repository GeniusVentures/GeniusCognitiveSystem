---
title: eval::evaluator

---

# eval::evaluator



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[eval::evaluator::SpecialistEvaluator](/python-reference/Classes/d0/d3a/classeval_1_1evaluator_1_1_specialist_evaluator/)**  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[parser](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-parser)**  |
| | **[required](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-required)**  |
| | **[True](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-true)**  |
| | **[help](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-help)**  |
| | **[args](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-args)**  |
| | **[project_root](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-project_root)**  |
| | **[evaluator](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-evaluator)**  |
| str | **[test_path](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-test_path)**  |
| list | **[test_samples](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-test_samples)**  |
| | **[line](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-line)**  |
| dict | **[results](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-results)**  |
| str | **[out_dir](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-out_dir)**  |
| | **[parents](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-parents)**  |
| | **[exist_ok](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-exist_ok)**  |
| | **[f](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-f)**  |
| | **[indent](/python-reference/Namespaces/df/d87/namespaceeval_1_1evaluator/#variable-indent)**  |

## Detailed Description




```
Per-specialist evaluation: perplexity, BLEU/ROUGE, latency via MLX.```



## Attributes Documentation

### variable parser

```python
parser =  argparse.ArgumentParser(description="Evaluate a specialist model");
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


### variable evaluator

```python
evaluator =  SpecialistEvaluator(project_root);
```


### variable test_path

```python
str test_path =  project_root / "data" / "specialists" / args.niche / "test.jsonl";
```


### variable test_samples

```python
list test_samples =  [];
```


### variable line

```python
line =  line.strip();
```


### variable results

```python
dict results =  {
        "niche": args.niche,
        "num_samples": len(test_samples),
        "accuracy": 0.0,
        "perplexity": 0.0,
        "latency_ms_per_token": 0.0,
    };
```


### variable out_dir

```python
str out_dir =  project_root / "artifacts" / "evaluations";
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