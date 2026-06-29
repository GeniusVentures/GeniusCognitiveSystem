---
title: training::tracker::ExperimentTracker

---

# training::tracker::ExperimentTracker





## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#function-__init__)**(self self, Optional project_root[Path] =None) |
| str | **[config_hash](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#function-config_hash)**(self self, [config](/python-reference/Namespaces/d9/de8/namespacetraining_1_1config/) config) |
| | **[start_run](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#function-start_run)**(self self, str niche_name, str variant ="default") |
| | **[log_params](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#function-log_params)**(self self, dict params) |
| | **[log_metrics](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#function-log_metrics)**(self self, dict metrics) |
| | **[end_run](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#function-end_run)**(self self) |
| list | **[list_runs](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#function-list_runs)**(self self) |
| list | **[compare_runs](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#function-compare_runs)**(self self) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_project_root](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#variable-_project_root)**  |
| str | **[_tracking_dir](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#variable-_tracking_dir)**  |
| bool | **[_active](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#variable-_active)**  |
| | **[_niche](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#variable-_niche)**  |
| | **[_variant](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#variable-_variant)**  |
| str | **[_run_id](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#variable-_run_id)**  |
| dict | **[_metrics](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#variable-_metrics)**  |
| | **[_params](/python-reference/Classes/de/d1d/classtraining_1_1tracker_1_1_experiment_tracker/#variable-_params)**  |

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    Optional project_root[Path] =None
)
```


### function config_hash

```python
str config_hash(
    self self,
    config config
)
```


### function start_run

```python
start_run(
    self self,
    str niche_name,
    str variant ="default"
)
```


### function log_params

```python
log_params(
    self self,
    dict params
)
```


### function log_metrics

```python
log_metrics(
    self self,
    dict metrics
)
```


### function end_run

```python
end_run(
    self self
)
```


### function list_runs

```python
list list_runs(
    self self
)
```


### function compare_runs

```python
list compare_runs(
    self self
)
```


## Protected Attributes Documentation

### variable _project_root

```python
_project_root =  project_root;
```


### variable _tracking_dir

```python
str _tracking_dir =  project_root / "artifacts" / "experiments";
```


### variable _active

```python
bool _active =  False;
```


### variable _niche

```python
_niche =  niche_name;
```


### variable _variant

```python
_variant =  variant;
```


### variable _run_id

```python
str _run_id =  f"{niche_name}_{variant}_{self.config_hash({})}";
```


### variable _metrics

```python
dict _metrics =  {};
```


### variable _params

```python
_params =  dict(params);
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700