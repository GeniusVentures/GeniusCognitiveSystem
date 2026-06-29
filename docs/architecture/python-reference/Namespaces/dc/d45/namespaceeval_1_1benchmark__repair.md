---
title: eval::benchmark_repair

---

# eval::benchmark_repair



 [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| dict | **[generate_repair_report](/python-reference/Namespaces/dc/d45/namespaceeval_1_1benchmark__repair/#function-generate_repair_report)**(str niche_name, dict gate_result, dict benchmark_results, Optional config[dict] =None, Optional previous_results[dict] =None) |
| Path | **[save_repair_report](/python-reference/Namespaces/dc/d45/namespaceeval_1_1benchmark__repair/#function-save_repair_report)**(str niche_name, dict report, Optional project_root[Path] =None) |
| bool | **[should_block_pipeline](/python-reference/Namespaces/dc/d45/namespaceeval_1_1benchmark__repair/#function-should_block_pipeline)**(dict gate_result) |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[logger](/python-reference/Namespaces/dc/d45/namespaceeval_1_1benchmark__repair/#variable-logger)**  |

## Detailed Description




```
Repair suggestion reports for below-threshold specialists (Plan 04-04 Task 2).

D-10 LOCKED DECISION: repair suggestions, NOT auto-mutation. The system advises;
the operator acts. After the 3rd consecutive failure the pipeline blocks
promotion and manual intervention is required.

T-04-19 mitigation: this module contains NO imports of any config-writing
function. The ``_generate_config_suggestions`` helper returns plain JSON-serializable
dictionaries only -- it never mutates a live config. Reports are read-only JSON
artifacts.
```


## Functions Documentation

### function generate_repair_report

```python
dict generate_repair_report(
    str niche_name,
    dict gate_result,
    dict benchmark_results,
    Optional config[dict] =None,
    Optional previous_results[dict] =None
)
```




```
Generate a structured repair suggestion report (D-10).

The system advises; the operator acts. This function NEVER mutates configs.

Args:
    niche_name: Specialist niche.
    gate_result: Output of ``Benchmarker.gate_check_benchmarks()`` (Plan 04-03).
    benchmark_results: Current benchmark results payload.
    config: Effective config dict with ``benchmarks`` block.
    previous_results: Optional prior-run results payload. When absent, the
        report is flagged ``no_baseline_available: True`` and only absolute
        threshold comparison is used.

Returns:
    Structured repair report dict (JSON-serializable).
```


### function save_repair_report

```python
Path save_repair_report(
    str niche_name,
    dict report,
    Optional project_root[Path] =None
)
```




```
Write a repair report to ``artifacts/repair_reports/{niche}_{timestamp}.json``.

WR-04: the filename timestamp is derived from ``report["report_id"]`` (set
by ``generate_repair_report``) so the filename is recoverable from the
report_id field. Falls back to a fresh timestamp only if report_id is
malformed.

Args:
    niche_name: Specialist niche.
    report: Report dict from ``generate_repair_report``.
    project_root: Project root.

Returns:
    Path to the written report.
```


### function should_block_pipeline

```python
bool should_block_pipeline(
    dict gate_result
)
```




```
Return True when the gate is blocking AND any benchmark has 3+ failures.

D-10: 3rd consecutive failure blocks pipeline promotion. The operator must
intervene manually -- the system never auto-fixes.

WR-05: D-08 also makes the SGFP4 regression check (quantized vs
unquantized-adapter) a MANDATORY gate dimension. A failed SGFP4 regression
(e.g. quantization destroyed 15% of accuracy) blocks the pipeline even
before any individual benchmark accumulates 3 consecutive failures. The
``needs_bootstrap: True`` placeholder result from
``Benchmarker._sgfp4_regression_check`` (first run, no unquantized
baseline) is treated as a pass -- only an explicit ``passed: False`` blocks.

Args:
    gate_result: Output of ``Benchmarker.gate_check_benchmarks()``.

Returns:
    True when the pipeline should be blocked.
```



## Attributes Documentation

### variable logger

```python
logger =  logging.getLogger(__name__);
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700