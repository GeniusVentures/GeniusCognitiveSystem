---
title: distill::synthetic

---

# distill::synthetic



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[distill::synthetic::SyntheticDataGenerator](/python-reference/Classes/d1/dbe/classdistill_1_1synthetic_1_1_synthetic_data_generator/)**  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| int | **[QUALITY_MIN_CHARS](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-quality_min_chars)**  |
| list | **[REFUSAL_PATTERNS](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-refusal_patterns)**  |
| | **[parser](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-parser)**  |
| | **[required](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-required)**  |
| | **[True](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-true)**  |
| | **[help](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-help)**  |
| | **[args](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-args)**  |
| | **[project_root](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-project_root)**  |
| | **[loader](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-loader)**  |
| | **[cfg](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-cfg)**  |
| | **[system_prompt](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-system_prompt)**  |
| | **[user_prompts](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-user_prompts)**  |
| | **[client](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-client)**  |
| | **[generator](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-generator)**  |
| | **[samples](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-samples)**  |

## Detailed Description




```
Synthetic data generation using multi-backend cascade-capable teacher models.```



## Attributes Documentation

### variable QUALITY_MIN_CHARS

```python
int QUALITY_MIN_CHARS =  200;
```


### variable REFUSAL_PATTERNS

```python
list REFUSAL_PATTERNS =  [
    r"\bI cannot\b",
    r"\bI['\u2019]m unable\b",
    r"\bas an AI\b",
    r"\bI don['\u2019]t have\b",
    r"\bI do not have\b",
    r"\bI am not able\b",
    r"\bI['\u2019]m not able\b",
    r"\bsorry.*cannot\b",
    r"\bcan['\u2019]t (?:help|assist|do that|generate|create|provide)\b",
];
```


### variable parser

```python
parser =  argparse.ArgumentParser(description="Generate synthetic data for a specialist niche");
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


### variable loader

```python
loader =  ConfigLoader(project_root);
```


### variable cfg

```python
cfg =  loader.get_effective_config(args.niche);
```


### variable system_prompt

```python
system_prompt =  cfg.get("system_prompt", f"You are a {args.niche} specialist.");
```


### variable user_prompts

```python
user_prompts =  cfg.get("synthetic_prompts", [f"Explain {args.niche} concepts in detail."]);
```


### variable client

```python
client =  TeacherClient(project_root);
```


### variable generator

```python
generator =  SyntheticDataGenerator(client, project_root, use_cascade=True);
```


### variable samples

```python
samples =  generator.generate_for_niche(args.niche, system_prompt, user_prompts);
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700