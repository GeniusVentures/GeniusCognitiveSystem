---
title: GNUS-NEO-SWARM/gnus-poc/distill/backends/openai_backend.py

---

# GNUS-NEO-SWARM/gnus-poc/distill/backends/openai_backend.py





## Namespaces

| Name           |
| -------------- |
| **[distill](/python-reference/Namespaces/dc/db8/namespacedistill/)**  |
| **[distill::backends](/python-reference/Namespaces/d7/dd3/namespacedistill_1_1backends/)**  |
| **[distill::backends::openai_backend](/python-reference/Namespaces/de/d8d/namespacedistill_1_1backends_1_1openai__backend/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[distill::backends::openai_backend::OpenAIBackend](/python-reference/Classes/d8/dea/classdistill_1_1backends_1_1openai__backend_1_1_open_a_i_backend/)**  |




## Source code

```python
"""OpenAI-compatible API backend using the ``openai`` Python SDK."""

from openai import OpenAI

from distill.backends.base import TeacherBackend


class OpenAIBackend(TeacherBackend):
    """Teacher backend that talks to any OpenAI-compatible endpoint.

    This wraps the official ``openai`` SDK and is used for endpoints whose
    ``apiType`` is ``"openai"`` — including the local LiteLLM proxy and
    direct OpenAI/DeepSeek API calls.
    """

    def __init__(self, endpoint_config: dict, model_id: str, api_key: str):
        super().__init__(endpoint_config, model_id, api_key)
        self._client = OpenAI(
            api_key=api_key,
            base_url=endpoint_config["url"],
        )

    @property
    def backend_type(self) -> str:
        return "openai"

    def generate(
        self,
        messages: list,
        max_tokens: int,
        temperature: float,
        **kwargs,
    ) -> dict:
        """Call the OpenAI Chat Completions endpoint and normalise the response.

        Returns:
            Uniform dict with ``content``, ``prompt_tokens``,
            ``completion_tokens``, and ``raw_response``.
        """
        response = self._client.chat.completions.create(
            model=self._model_id,
            messages=messages,
            max_tokens=max_tokens,
            temperature=temperature,
            **kwargs,
        )

        choice = response.choices[0]
        usage = response.usage

        return {
            "content": choice.message.content,
            "prompt_tokens": usage.prompt_tokens,
            "completion_tokens": usage.completion_tokens,
            "raw_response": response,
        }
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700
