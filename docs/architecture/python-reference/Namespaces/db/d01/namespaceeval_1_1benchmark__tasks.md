---
title: eval::benchmark_tasks

---

# eval::benchmark_tasks



 [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| TaskManager | **[create_task_manager](/python-reference/Namespaces/db/d01/namespaceeval_1_1benchmark__tasks/#function-create_task_manager)**(Path|None config_dir =None) |
| None | **[check](/python-reference/Namespaces/db/d01/namespaceeval_1_1benchmark__tasks/#function-check)**(str name, bool condition, str detail ="") |

## Attributes

|                | Name           |
| -------------- | -------------- |
| Path | **[BENCHMARKS_CONFIG_DIR](/python-reference/Namespaces/db/d01/namespaceeval_1_1benchmark__tasks/#variable-benchmarks_config_dir)**  |
| int | **[passed](/python-reference/Namespaces/db/d01/namespaceeval_1_1benchmark__tasks/#variable-passed)**  |
| int | **[failed](/python-reference/Namespaces/db/d01/namespaceeval_1_1benchmark__tasks/#variable-failed)**  |
| TaskManager | **[tm](/python-reference/Namespaces/db/d01/namespaceeval_1_1benchmark__tasks/#variable-tm)**  |
| TaskManager | **[entry](/python-reference/Namespaces/db/d01/namespaceeval_1_1benchmark__tasks/#variable-entry)**  |
| TaskManager | **[cfg](/python-reference/Namespaces/db/d01/namespaceeval_1_1benchmark__tasks/#variable-cfg)**  |
| dict | **[metric_names](/python-reference/Namespaces/db/d01/namespaceeval_1_1benchmark__tasks/#variable-metric_names)**  |

## Detailed Description




```
TaskManager setup for custom lm-eval benchmark tasks.

Per Phase 04-02 Task 1: PubMedQA and BIGPATENT are not natively supported by
lm-eval-harness in the format required by the POC. This module registers the
custom YAML task definitions in ``config/benchmarks/`` with an
``lm_eval.tasks.TaskManager`` so they are available to ``simple_evaluate()``.

The custom YAMLs live alongside the per-benchmark config YAMLs. Files without a
``task:`` field (the per-benchmark configs and ``specialist_mapping.yaml``) are
silently ignored by TaskManager — only files with a ``task:`` key are registered.

Threat mitigations:
- T-04-06: ``include_path`` is project-internal and YAMLs are parsed with
  ``yaml.safe_load`` by lm-eval internally. No arbitrary code execution.
```


## Functions Documentation

### function create_task_manager

```python
TaskManager create_task_manager(
    Path|None config_dir =None
)
```




```
Create an ``lm_eval.tasks.TaskManager`` with custom benchmark YAMLs registered.

The ``include_path`` parameter adds every ``*.yaml`` in ``config_dir`` that
defines a ``task:`` field to lm-eval's task registry. Files without a
``task:`` key (per-benchmark config YAMLs, ``specialist_mapping.yaml``) are
silently skipped by TaskManager.

Args:
    config_dir: Directory containing custom task YAML files. Defaults to
        ``<project_root>/config/benchmarks/``.

Returns:
    Configured ``TaskManager`` instance with custom tasks registered.

Raises:
    FileNotFoundError: If ``config_dir`` does not exist.
```


### function check

```python
None check(
    str name,
    bool condition,
    str detail =""
)
```



## Attributes Documentation

### variable BENCHMARKS_CONFIG_DIR

```python
Path BENCHMARKS_CONFIG_DIR =  _PROJECT_ROOT / "config" / "benchmarks";
```


### variable passed

```python
int passed =  0;
```


### variable failed

```python
int failed =  0;
```


### variable tm

```python
TaskManager tm =  create_task_manager();
```


### variable entry

```python
TaskManager entry =  tm.task_index["pubmedqa"];
```


### variable cfg

```python
TaskManager cfg =  entry.cfg if hasattr(entry, "cfg") else entry;
```


### variable metric_names

```python
dict metric_names =  {m.get("metric") for m in cfg.get("metric_list", [])};
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700