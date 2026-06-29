---
title: GNUS-NEO-SWARM/gnus-poc/distill/synthetic.py

---

# GNUS-NEO-SWARM/gnus-poc/distill/synthetic.py





## Namespaces

| Name           |
| -------------- |
| **[distill](/python-reference/Namespaces/dc/db8/namespacedistill/)**  |
| **[distill::synthetic](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[distill::synthetic::SyntheticDataGenerator](/python-reference/Classes/d1/dbe/classdistill_1_1synthetic_1_1_synthetic_data_generator/)**  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| int | **[QUALITY_MIN_CHARS](/python-reference/Files/d8/d49/synthetic_8py/#variable-quality_min_chars)**  |
| list | **[REFUSAL_PATTERNS](/python-reference/Files/d8/d49/synthetic_8py/#variable-refusal_patterns)**  |
| | **[parser](/python-reference/Files/d8/d49/synthetic_8py/#variable-parser)**  |
| | **[required](/python-reference/Files/d8/d49/synthetic_8py/#variable-required)**  |
| | **[True](/python-reference/Files/d8/d49/synthetic_8py/#variable-true)**  |
| | **[help](/python-reference/Files/d8/d49/synthetic_8py/#variable-help)**  |
| | **[args](/python-reference/Files/d8/d49/synthetic_8py/#variable-args)**  |
| | **[project_root](/python-reference/Files/d8/d49/synthetic_8py/#variable-project_root)**  |
| | **[loader](/python-reference/Files/d8/d49/synthetic_8py/#variable-loader)**  |
| | **[cfg](/python-reference/Files/d8/d49/synthetic_8py/#variable-cfg)**  |
| | **[system_prompt](/python-reference/Files/d8/d49/synthetic_8py/#variable-system_prompt)**  |
| | **[user_prompts](/python-reference/Files/d8/d49/synthetic_8py/#variable-user_prompts)**  |
| | **[client](/python-reference/Files/d8/d49/synthetic_8py/#variable-client)**  |
| | **[generator](/python-reference/Files/d8/d49/synthetic_8py/#variable-generator)**  |
| | **[samples](/python-reference/Files/d8/d49/synthetic_8py/#variable-samples)**  |



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



## Source code

```python
"""Synthetic data generation using multi-backend cascade-capable teacher models."""

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Optional

from config.loader import ConfigLoader
from distill.cascade import _DOMAIN_MAP
from distill.teacher import TeacherClient
from distill.teacher_errors import SyntheticDataError


QUALITY_MIN_CHARS = 200

REFUSAL_PATTERNS = [
    r"\bI cannot\b",
    r"\bI['\u2019]m unable\b",
    r"\bas an AI\b",
    r"\bI don['\u2019]t have\b",
    r"\bI do not have\b",
    r"\bI am not able\b",
    r"\bI['\u2019]m not able\b",
    r"\bsorry.*cannot\b",
    r"\bcan['\u2019]t (?:help|assist|do that|generate|create|provide)\b",
]

_refusal_re = re.compile("|".join(REFUSAL_PATTERNS), re.IGNORECASE)


class SyntheticDataGenerator:
    def __init__(
        self,
        teacher_client: TeacherClient,
        project_root: Optional[Path] = None,
        use_cascade: bool = True,
        domain: str = "encyclopedic",
    ):
        self._client = teacher_client
        self._use_cascade = use_cascade
        self._default_domain = domain
        if project_root is None:
            project_root = Path(__file__).resolve().parent.parent
        self._project_root = project_root

    def generate_for_niche(
        self,
        niche_name: str,
        system_prompt: str,
        user_prompts: list,
        num_samples: int = 500,
        keywords: Optional[list] = None,
    ) -> list:
        if not user_prompts:
            return []

        domain = _DOMAIN_MAP.get(niche_name, self._default_domain)

        results = []
        repeats = (num_samples // len(user_prompts)) + 1
        for i, user_prompt in enumerate(user_prompts * repeats):
            if len(results) >= num_samples:
                break

            messages = [
                {"role": "system", "content": system_prompt},
                {"role": "user", "content": user_prompt},
            ]

            try:
                if self._use_cascade:
                    response = self._client.generate_with_cascade(messages, domain=domain)
                else:
                    response = self._client.generate(model_name=None, messages=messages)
                content = response.choices[0].message.content

                if self._passes_quality(content, keywords):
                    results.append({
                        "text": content,
                        "source": "synthetic_deepseek_v4_pro",
                        "niche": niche_name,
                        "prompt": user_prompt,
                    })
            except Exception as e:
                raise SyntheticDataError(
                    f"Failed to generate sample {i} for niche '{niche_name}': {e}"
                ) from e

        return results

    def _passes_quality(self, text: str, keywords: Optional[list] = None) -> bool:
        if len(text) < QUALITY_MIN_CHARS:
            return False

        if _refusal_re.search(text):
            return False

        if keywords:
            text_lower = text.lower()
            if not any(kw.lower() in text_lower for kw in keywords):
                return False

        return True

    def save_to_jsonl(self, samples: list, output_path: Path):
        output_path.parent.mkdir(parents=True, exist_ok=True)
        with output_path.open("w") as f:
            for sample in samples:
                f.write(json.dumps(sample) + "\n")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate synthetic data for a specialist niche")
    parser.add_argument("--niche", required=True, help="Specialist niche name")
    args = parser.parse_args()

    project_root = Path(__file__).resolve().parent.parent
    loader = ConfigLoader(project_root)
    cfg = loader.get_effective_config(args.niche)
    system_prompt = cfg.get("system_prompt", f"You are a {args.niche} specialist.")
    user_prompts = cfg.get("synthetic_prompts", [f"Explain {args.niche} concepts in detail."])
    client = TeacherClient(project_root)
    generator = SyntheticDataGenerator(client, project_root, use_cascade=True)
    samples = generator.generate_for_niche(args.niche, system_prompt, user_prompts)
    generator.save_to_jsonl(samples, project_root / "artifacts" / "synthetic" / f"{args.niche}.jsonl")
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700
