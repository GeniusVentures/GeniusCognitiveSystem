---
title: GNUS-NEO-SWARM/gnus-poc/eval/benchmark_tasks.py

---

# GNUS-NEO-SWARM/gnus-poc/eval/benchmark_tasks.py





## Namespaces

| Name           |
| -------------- |
| **[eval](/python-reference/Namespaces/dd/df7/namespaceeval/)**  |
| **[eval::benchmark_tasks](/python-reference/Namespaces/db/d01/namespaceeval_1_1benchmark__tasks/)**  |

## Functions

|                | Name           |
| -------------- | -------------- |
| TaskManager | **[create_task_manager](/python-reference/Files/d1/de5/benchmark__tasks_8py/#function-create_task_manager)**(Path|None config_dir =None) |
| None | **[check](/python-reference/Files/d1/de5/benchmark__tasks_8py/#function-check)**(str name, bool condition, str detail ="") |

## Attributes

|                | Name           |
| -------------- | -------------- |
| Path | **[BENCHMARKS_CONFIG_DIR](/python-reference/Files/d1/de5/benchmark__tasks_8py/#variable-benchmarks_config_dir)**  |
| int | **[passed](/python-reference/Files/d1/de5/benchmark__tasks_8py/#variable-passed)**  |
| int | **[failed](/python-reference/Files/d1/de5/benchmark__tasks_8py/#variable-failed)**  |
| TaskManager | **[tm](/python-reference/Files/d1/de5/benchmark__tasks_8py/#variable-tm)**  |
| TaskManager | **[entry](/python-reference/Files/d1/de5/benchmark__tasks_8py/#variable-entry)**  |
| TaskManager | **[cfg](/python-reference/Files/d1/de5/benchmark__tasks_8py/#variable-cfg)**  |
| dict | **[metric_names](/python-reference/Files/d1/de5/benchmark__tasks_8py/#variable-metric_names)**  |


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



## Source code

```python
"""TaskManager setup for custom lm-eval benchmark tasks.

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
"""

from __future__ import annotations

from pathlib import Path

from lm_eval.tasks import TaskManager


# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------

# Project layout: gnus-poc/eval/benchmark_tasks.py -> gnus-poc/config/benchmarks/
_PROJECT_ROOT = Path(__file__).resolve().parent.parent
BENCHMARKS_CONFIG_DIR: Path = _PROJECT_ROOT / "config" / "benchmarks"


# ---------------------------------------------------------------------------
# Public API
# ---------------------------------------------------------------------------

def create_task_manager(
    config_dir: Path | None = None,
) -> TaskManager:
    """Create an ``lm_eval.tasks.TaskManager`` with custom benchmark YAMLs registered.

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
    """
    resolved_dir = config_dir if config_dir is not None else BENCHMARKS_CONFIG_DIR

    if not resolved_dir.is_dir():
        raise FileNotFoundError(
            f"Benchmarks config directory not found: {resolved_dir}"
        )

    # include_path registers every YAML in the directory that has a `task:` key.
    # lm-eval logs (does not error on) YAMLs without `task:` — safe to mix
    # custom task YAMLs and per-benchmark config YAMLs in the same directory.
    return TaskManager(include_path=str(resolved_dir))


# ---------------------------------------------------------------------------
# Self-test (run: python eval/benchmark_tasks.py)
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    import sys

    passed = 0
    failed = 0

    def check(name: str, condition: bool, detail: str = "") -> None:
        global passed, failed
        if condition:
            passed += 1
            print(f"  PASS  {name}")
        else:
            failed += 1
            print(f"  FAIL  {name}{' — ' + detail if detail else ''}")

    try:
        tm = create_task_manager()
        check("create_task_manager() returns TaskManager", True)
    except Exception as exc:  # pragma: no cover - manual self-test
        check("create_task_manager() returns TaskManager", False, str(exc))
        sys.exit(1)

    # PubMedQA custom task registered
    check("pubmedqa registered", "pubmedqa" in tm.task_index)
    if "pubmedqa" in tm.task_index:
        entry = tm.task_index["pubmedqa"]
        cfg = entry.cfg if hasattr(entry, "cfg") else entry
        check(
            "pubmedqa dataset_path",
            cfg.get("dataset_path") == "qiaojin/PubMedQA",
            str(cfg.get("dataset_path")),
        )
        check(
            "pubmedqa output_type multiple_choice",
            cfg.get("output_type") == "multiple_choice",
        )
        check(
            "pubmedqa 3-way choice",
            cfg.get("doc_to_choice") == ["yes", "no", "maybe"],
            str(cfg.get("doc_to_choice")),
        )

    # BIGPATENT custom task registered
    check("bigpatent registered", "bigpatent" in tm.task_index)
    if "bigpatent" in tm.task_index:
        entry = tm.task_index["bigpatent"]
        cfg = entry.cfg if hasattr(entry, "cfg") else entry
        check(
            "bigpatent dataset_path",
            cfg.get("dataset_path") == "big_patent",
            str(cfg.get("dataset_path")),
        )
        check(
            "bigpatent output_type generate_until",
            cfg.get("output_type") == "generate_until",
        )
        metric_names = {m.get("metric") for m in cfg.get("metric_list", [])}
        check("bigpatent has rouge1", "rouge1" in metric_names)
        check("bigpatent has rougeL", "rougeL" in metric_names)

    print(f"\n  {passed} passed, {failed} failed")
    sys.exit(0 if failed == 0 else 1)
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700
