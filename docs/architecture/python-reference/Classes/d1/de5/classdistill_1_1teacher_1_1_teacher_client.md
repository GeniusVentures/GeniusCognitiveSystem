---
title: distill::teacher::TeacherClient

---

# distill::teacher::TeacherClient



 [More...](#detailed-description)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-__init__)**(self self, Optional config_path[Path] =None, Optional project_root[Path] =None) |
| | **[reset_budget](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-reset_budget)**(self self) |
| | **[generate](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-generate)**(self self, Optional model_name[str] =None, messages messages =None, ** kwargs) |
| | **[generate_with_logprobs](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-generate_with_logprobs)**(self self, Optional model_name[str] =None, messages messages =None, ** kwargs) |
| | **[generate_with_cascade](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-generate_with_cascade)**(self self, messages messages, domain domain ="encyclopedic", ** kwargs) |
| | **[total_cost](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-total_cost)**(self self) |
| | **[budget_cap](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-budget_cap)**(self self) |
| | **[circuit_open](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-circuit_open)**(self self) |
| | **[call_count](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-call_count)**(self self) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| str | **[_resolve_api_key](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-_resolve_api_key)**(str endpoint_name, str api_type) |
| | **[_load_config](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-_load_config)**(self self, config_path config_path) |
| | **[_get_or_create_backend](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-_get_or_create_backend)**(self self, str endpoint_name) |
| | **[_resolve_backend](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-_resolve_backend)**(self self, str model_name) |
| float | **[_estimate_cost](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-_estimate_cost)**(self self, int prompt_tokens, int completion_tokens) |
| | **[_log_cost](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-_log_cost)**(self self, str model_name, int prompt_tokens, int completion_tokens, float cost) |
| | **[_log_error](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-_log_error)**(self self, str error_type, Optional status_code[int], str detail) |
| | **[_load_budget_state](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-_load_budget_state)**(self self) |
| | **[_save_budget_state](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-_save_budget_state)**(self self) |
| | **[_check_circuit](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-_check_circuit)**(self self) |
| | **[_check_budget](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-_check_budget)**(self self) |
| bool | **[_is_retryable](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-_is_retryable)**(self self, Exception exception) |
| | **[_call_api](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#function-_call_api)**(self self, str model_name, messages messages, ** kwargs) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_project_root](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_project_root)**  |
| | **[_config](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_config)**  |
| | **[_models](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_models)**  |
| | **[_default_max_tokens](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_default_max_tokens)**  |
| | **[_default_temperature](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_default_temperature)**  |
| | **[_max_retries](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_max_retries)**  |
| | **[_backoff_base](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_backoff_base)**  |
| | **[_budget_cap](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_budget_cap)**  |
| | **[_max_consecutive_failures](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_max_consecutive_failures)**  |
| | **[_circuit_recovery_timeout](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_circuit_recovery_timeout)**  |
| dict | **[_endpoint_registry](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_endpoint_registry)**  |
| dict | **[_backends](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_backends)**  |
| | **[_cascade](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_cascade)**  |
| float | **[_total_cost](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_total_cost)**  |
| int | **[_budget_version](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_budget_version)**  |
| int | **[_call_count](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_call_count)**  |
| int | **[_consecutive_failures](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_consecutive_failures)**  |
| bool | **[_circuit_open](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_circuit_open)**  |
| | **[_circuit_opened_at](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_circuit_opened_at)**  |
| bool | **[_circuit_half_open](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_circuit_half_open)**  |
| str | **[_cost_log_path](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_cost_log_path)**  |
| str | **[_error_log_path](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_error_log_path)**  |
| str | **[_budget_state_path](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/#variable-_budget_state_path)**  |

## Detailed Description

```python
class distill::teacher::TeacherClient;
```




```
Multi-backend teacher API client.

Builds a backend registry from ``config/pipeline.yaml`` endpoints and
dispatches each ``generate()`` call to the correct backend based on the
model's endpoint ``apiType``.

Backends are constructed lazily on first use so that test code can inject
mock backends via ``client._backends`` without triggering real SDK imports.
```

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    Optional config_path[Path] =None,
    Optional project_root[Path] =None
)
```


### function reset_budget

```python
reset_budget(
    self self
)
```




```
Reset cumulative spend to zero and persist the change.

Used by the pipeline runner when the ``--reset-budget`` CLI flag
is passed.
```


### function generate

```python
generate(
    self self,
    Optional model_name[str] =None,
    messages messages =None,
    ** kwargs
)
```




```
Generate a completion through the appropriate backend.

Args:
    model_name: Model key from the ``models`` config block.  If ``None``,
        defaults to ``teacher.level1`` from pipeline.yaml.
    messages: List of message dicts (OpenAI format).
    **kwargs: Extra parameters forwarded to the backend.

Returns:
    ``_ResponseWrapper`` with ``.choices[0].message.content`` and ``.usage``.
```


### function generate_with_logprobs

```python
generate_with_logprobs(
    self self,
    Optional model_name[str] =None,
    messages messages =None,
    ** kwargs
)
```




```
Generate with log-probabilities (OpenAI-compatible endpoints only).

Args:
    model_name: Model key (defaults to ``teacher.level1``).
    messages: List of message dicts.
    **kwargs: Extra parameters.

Returns:
    ``_ResponseWrapper`` with logprobs data.
```


### function generate_with_cascade

```python
generate_with_cascade(
    self self,
    messages messages,
    domain domain ="encyclopedic",
    ** kwargs
)
```




```
Generate a completion using the multi-teacher cascade.

Routes through ``TeacherCascade.execute()``: Level 1 always runs;
Level 2 is invoked only when Level 1 confidence is below threshold
and the best Level 2 teacher is selected from the benchmark table.

Args:
    messages: List of message dicts (OpenAI format).
    domain: Specialist niche name (e.g. ``"code"``, ``"medical"``).
        Defaults to ``"encyclopedic"``.
    **kwargs: Extra parameters forwarded to each teacher call.

Returns:
    ``_ResponseWrapper`` with ``.choices[0].message.content`` set to
    the cascade's final content.  The raw response payload is the
    cascade result dict (for logging / inspection).
```


### function total_cost

```python
total_cost(
    self self
)
```


### function budget_cap

```python
budget_cap(
    self self
)
```


### function circuit_open

```python
circuit_open(
    self self
)
```


### function call_count

```python
call_count(
    self self
)
```


## Protected Functions Documentation

### function _resolve_api_key

```python
static str _resolve_api_key(
    str endpoint_name,
    str api_type
)
```




```
Resolve the API key for an endpoint.

Priority:
1. ``LITELLM_API_KEY`` env var (for LiteLLM proxy endpoints)
2. ``{ENDPOINT_NAME_UPPER}_API_KEY`` env var
3. ``{API_TYPE_UPPER}_API_KEY`` env var (e.g. ``ANTHROPIC_API_KEY``)

Raises:
    TeacherConfigError: If no API key is found.
```


### function _load_config

```python
_load_config(
    self self,
    config_path config_path
)
```


### function _get_or_create_backend

```python
_get_or_create_backend(
    self self,
    str endpoint_name
)
```




```
Return (possibly creating) the backend instance for an endpoint.

Backends are created lazily so that tests may inject mocks into
``self._backends`` before any real SDK client is constructed.
```


### function _resolve_backend

```python
_resolve_backend(
    self self,
    str model_name
)
```




```
Look up the backend instance for a model name.

Args:
    model_name: Key in the ``models`` config block (e.g. ``"deepseek-v4-fast"``).

Returns:
    A ``TeacherBackend`` instance.

Raises:
    TeacherConfigError: If the model or its endpoint is unknown.
```


### function _estimate_cost

```python
float _estimate_cost(
    self self,
    int prompt_tokens,
    int completion_tokens
)
```


### function _log_cost

```python
_log_cost(
    self self,
    str model_name,
    int prompt_tokens,
    int completion_tokens,
    float cost
)
```


### function _log_error

```python
_log_error(
    self self,
    str error_type,
    Optional status_code[int],
    str detail
)
```


### function _load_budget_state

```python
_load_budget_state(
    self self
)
```




```
Load cumulative spend from ``artifacts/.budget_state.json``.

Budget state file format::

    {
        "cumulative_cost_usd": 1.234,
        "budget_cap_usd": 5.0,
        "last_updated": "2026-06-19T12:00:00+00:00",
        "version": 1
    }

If the file does not exist the budget starts at ``0.0``.
The budget state file can be edited manually — it is a soft
cost-control limit, not a security boundary (see T-04-01).
```


### function _save_budget_state

```python
_save_budget_state(
    self self
)
```




```
Persist current cumulative spend to ``artifacts/.budget_state.json``.

Called after every successful API call that adds cost.  Creates
parent directories if they do not exist.
```


### function _check_circuit

```python
_check_circuit(
    self self
)
```




```
Gate API calls through a half-open circuit breaker.

**Closed:**  calls proceed normally.
**Open:**    calls are blocked for ``recovery_timeout`` seconds.
             After the timeout elapses the circuit transitions to
             *half-open* — the next call is allowed as a probe.
**Half-open:** a single probe call is permitted.  If it succeeds
             the circuit closes.  If it fails the circuit re-opens
             with a fresh recovery timer.

Raises:
    CircuitBreakerOpenError: When the circuit is open and the
        recovery timeout has not elapsed.
```


### function _check_budget

```python
_check_budget(
    self self
)
```




```
Raise ``BudgetExceededError`` when cumulative spend hits the cap.

Budget enforcement reads the persisted total from disk on startup
(see ``_load_budget_state``), so the cap applies across runs.
```


### function _is_retryable

```python
bool _is_retryable(
    self self,
    Exception exception
)
```


### function _call_api

```python
_call_api(
    self self,
    str model_name,
    messages messages,
    ** kwargs
)
```




```
Execute an API call through the correct backend with retry + circuit breaker.

Circuit breaker state machine:

* **Closed** → calls proceed; after ``failure_threshold`` consecutive
  failures the circuit **opens** with a timestamp.
* **Open** → calls are blocked for ``recovery_timeout`` seconds.
* **Half-open** → one probe call is allowed.  Success **closes** the
  circuit.  Failure **re-opens** it with a fresh recovery timer.

Args:
    model_name: Key from the ``models`` config block.
    messages: List of message dicts (OpenAI format).
    **kwargs: Passed to ``backend.generate()`` (max_tokens, temperature, etc.).

Returns:
    ``_ResponseWrapper`` with ``.choices[0].message.content`` and ``.usage``.
```


## Protected Attributes Documentation

### variable _project_root

```python
_project_root =  project_root;
```


### variable _config

```python
_config =  self._load_config(config_path);
```


### variable _models

```python
_models =  models_cfg;
```


### variable _default_max_tokens

```python
_default_max_tokens =  int(teacher_cfg.get("max_tokens", 4096));
```


### variable _default_temperature

```python
_default_temperature =  float(teacher_cfg.get("temperature", 0.7));
```


### variable _max_retries

```python
_max_retries =  int(teacher_cfg.get("max_retries", 3));
```


### variable _backoff_base

```python
_backoff_base =  float(teacher_cfg.get("backoff_base_seconds", 2.0));
```


### variable _budget_cap

```python
_budget_cap =  float(teacher_cfg.get("budget_cap_usd", 5.0));
```


### variable _max_consecutive_failures

```python
_max_consecutive_failures =  int(
            teacher_cfg.get("circuit_breaker_failure_threshold", 5)
        );
```


### variable _circuit_recovery_timeout

```python
_circuit_recovery_timeout =  float(
            teacher_cfg.get("circuit_breaker_recovery_timeout", 60)
        );
```


### variable _endpoint_registry

```python
dict _endpoint_registry =  {};
```


### variable _backends

```python
dict _backends =  {};
```


### variable _cascade

```python
_cascade =  TeacherCascade(
            teacher_client=self,
            benchmark_table=teacher_benchmark,
            level1_model=level1_model,
            confidence_threshold=confidence_threshold,
        );
```


### variable _total_cost

```python
float _total_cost =  0.0;
```


### variable _budget_version

```python
int _budget_version =  1;
```


### variable _call_count

```python
int _call_count =  0;
```


### variable _consecutive_failures

```python
int _consecutive_failures =  0;
```


### variable _circuit_open

```python
bool _circuit_open =  False;
```


### variable _circuit_opened_at

```python
_circuit_opened_at =  None;
```


### variable _circuit_half_open

```python
bool _circuit_half_open =  False;
```


### variable _cost_log_path

```python
str _cost_log_path =  project_root / "artifacts" / "api_cost.jsonl";
```


### variable _error_log_path

```python
str _error_log_path =  project_root / "artifacts" / "api_errors.jsonl";
```


### variable _budget_state_path

```python
str _budget_state_path =  project_root / "artifacts" / ".budget_state.json";
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700