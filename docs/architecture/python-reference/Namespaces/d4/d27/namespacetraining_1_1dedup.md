---
title: training::dedup

---

# training::dedup



 [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| dict | **[compute_overlap_matrix](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#function-compute_overlap_matrix)**(dict samples_by_niche, int num_perm =128) |
| list | **[deduplicate_within_niche](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#function-deduplicate_within_niche)**(list samples, float jaccard_threshold =0.8, int num_perm =128) |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[parser](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-parser)**  |
| | **[required](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-required)**  |
| | **[True](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-true)**  |
| | **[help](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-help)**  |
| | **[args](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-args)**  |
| | **[project_root](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-project_root)**  |
| str | **[input_path](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-input_path)**  |
| list | **[samples](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-samples)**  |
| | **[line](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-line)**  |
| list | **[deduped](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-deduped)**  |
| | **[removed](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-removed)**  |
| str | **[out_dir](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-out_dir)**  |
| | **[parents](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-parents)**  |
| | **[exist_ok](/python-reference/Namespaces/d4/d27/namespacetraining_1_1dedup/#variable-exist_ok)**  |

## Detailed Description




```
Cross-niche deduplication using MinHash LSH.```


## Functions Documentation

### function compute_overlap_matrix

```python
dict compute_overlap_matrix(
    dict samples_by_niche,
    int num_perm =128
)
```


### function deduplicate_within_niche

```python
list deduplicate_within_niche(
    list samples,
    float jaccard_threshold =0.8,
    int num_perm =128
)
```



## Attributes Documentation

### variable parser

```python
parser =  argparse.ArgumentParser(description="Deduplicate synthetic data for a specialist niche");
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


### variable input_path

```python
str input_path =  project_root / "artifacts" / "synthetic" / f"{args.niche}.jsonl";
```


### variable samples

```python
list samples =  [];
```


### variable line

```python
line =  line.strip();
```


### variable deduped

```python
list deduped =  deduplicate_within_niche(samples);
```


### variable removed

```python
removed =  len(samples) - len(deduped);
```


### variable out_dir

```python
str out_dir =  project_root / "artifacts" / "dedup";
```


### variable parents

```python
parents;
```


### variable exist_ok

```python
exist_ok;
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700