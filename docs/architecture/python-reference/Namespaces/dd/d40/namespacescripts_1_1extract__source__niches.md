---
title: scripts::extract_source_niches

---

# scripts::extract_source_niches



 [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| | **[extract_source_based_niches](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/#function-extract_source_based_niches)**([sample_size](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/#variable-sample_size) sample_size =50000) |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[PROJECT_ROOT](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/#variable-project_root)**  |
| | **[exist_ok](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/#variable-exist_ok)**  |
| dict | **[TARGET_NICHES](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/#variable-target_niches)**  |
| | **[viable_niches](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/#variable-viable_niches)**  |
| | **[source_counts](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/#variable-source_counts)**  |
| | **[all_niche_samples](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/#variable-all_niche_samples)**  |
| | **[sample_size](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/#variable-sample_size)**  |
| dict | **[output_data](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/#variable-output_data)**  |
| | **[output_path](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/#variable-output_path)**  |
| | **[f](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/#variable-f)**  |
| | **[indent](/python-reference/Namespaces/dd/d40/namespacescripts_1_1extract__source__niches/#variable-indent)**  |

## Detailed Description




```
Source-based niche extraction from Common Pile
More reliable than clustering for creating distinct specialists
```


## Functions Documentation

### function extract_source_based_niches

```python
extract_source_based_niches(
    sample_size sample_size =50000
)
```




```
Extract niches directly from source labels```



## Attributes Documentation

### variable PROJECT_ROOT

```python
PROJECT_ROOT =  Path(__file__).resolve().parent.parent.parent;
```


### variable exist_ok

```python
exist_ok;
```


### variable TARGET_NICHES

```python
dict TARGET_NICHES;
```


### variable viable_niches

```python
viable_niches;
```


### variable source_counts

```python
source_counts;
```


### variable all_niche_samples

```python
all_niche_samples;
```


### variable sample_size

```python
sample_size;
```


### variable output_data

```python
dict output_data =  {
        'viable_niches': viable_niches,
        'all_sources': source_counts,
        'extraction_config': {
            'sample_size': 50000,
            'target_niches': TARGET_NICHES
        }
    };
```


### variable output_path

```python
output_path =  str(PROJECT_ROOT / 'data' / 'analysis' / 'source_based_niches.json');
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