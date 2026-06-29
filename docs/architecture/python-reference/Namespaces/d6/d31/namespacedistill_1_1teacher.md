---
title: distill::teacher

---

# distill::teacher



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[distill::teacher::TeacherClient](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/)**  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| dict | **[HTTP_NON_RETRYABLE](/python-reference/Namespaces/d6/d31/namespacedistill_1_1teacher/#variable-http_non_retryable)**  |
| int | **[HTTP_RATE_LIMIT](/python-reference/Namespaces/d6/d31/namespacedistill_1_1teacher/#variable-http_rate_limit)**  |
| tuple | **[NON_RETRYABLE_EXCEPTIONS](/python-reference/Namespaces/d6/d31/namespacedistill_1_1teacher/#variable-non_retryable_exceptions)**  |

## Detailed Description




```
Multi-backend teacher API client with cost controls, retry, and circuit breaker.

Dispatches teacher calls to the correct backend (OpenAI or Anthropic) based on the
model's configured endpoint ``apiType``.  All backends produce a uniform response
wrapper so callers receive the same interface regardless of backend.
```



## Attributes Documentation

### variable HTTP_NON_RETRYABLE

```python
dict HTTP_NON_RETRYABLE =  {400, 401, 402, 403, 404, 405, 422};
```


### variable HTTP_RATE_LIMIT

```python
int HTTP_RATE_LIMIT =  429;
```


### variable NON_RETRYABLE_EXCEPTIONS

```python
tuple NON_RETRYABLE_EXCEPTIONS =  (
    BudgetExceededError,
    CircuitBreakerOpenError,
    TeacherConfigError,
    BackendNotFoundError,
);
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700