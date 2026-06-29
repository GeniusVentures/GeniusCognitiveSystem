---
title: distill::cascade

---

# distill::cascade



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[distill::cascade::CascadeResult](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/)**  |
| class | **[distill::cascade::TeacherCascade](/python-reference/Classes/d6/d79/classdistill_1_1cascade_1_1_teacher_cascade/)**  |

## Functions

|                | Name           |
| -------------- | -------------- |
| float | **[compute_logprob_confidence](/python-reference/Namespaces/d9/df9/namespacedistill_1_1cascade/#function-compute_logprob_confidence)**(response response) |

## Detailed Description




```
Multi-teacher cascade orchestrator with benchmark-routed Level 2 escalation.

Implements a confidence-gated teacher escalation flow:

1. **Level 1 (always):** The fast, cheap teacher runs first for every request.
   When its logprobs mean confidence is at or above the configured threshold,
   the result is returned immediately — no Level 2 invocation.

2. **Benchmark-Routed Level 2:** When Level 1 confidence is below threshold,
   the best Level 2 teacher for the detected domain is selected from a
   pre-configured benchmark table.  If that teacher still falls below
   threshold, the next-highest-scoring Level 2 teacher is tried.

3. **Best-Effort Return:** The cascade never fails silently.  If all Level 2
   teachers produce below-threshold confidence, the highest-confidence result
   among them is returned.  If every teacher raises an exception,
   ``TeacherConfigError`` is raised with diagnostic details.

The benchmark table is loaded from ``teacher_benchmark`` in ``pipeline.yaml``
and maps domain keys to ``{model_name: strength_score}`` dicts.  Model names
must match keys in the ``models`` config block.
```


## Functions Documentation

### function compute_logprob_confidence

```python
float compute_logprob_confidence(
    response response
)
```




```
Compute mean token probability (confidence) from a response with logprobs.

Extracts logprobs from an OpenAI-compatible response format::

    response.choices[0].logprobs.content[].token_logprob

When the response is a ``_ResponseWrapper``, logprobs are accessed
via ``response._raw_response``.  Falls back to accessing ``response``
directly for test mocks that set logprobs on the outer object.

Returns the exponential of the mean log probability, yielding a value in
``[0.0, 1.0]``.  Returns ``0.0`` when logprobs data is missing or the
response structure is unexpected.

Args:
    response: A ``_ResponseWrapper`` returned by
        ``TeacherClient.generate_with_logprobs()``, or a mock with
        ``.choices[0].logprobs.content[]`` directly accessible.

Returns:
    Confidence score as a float between 0.0 and 1.0.
```






-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700