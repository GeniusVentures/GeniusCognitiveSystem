---
title: training::tokenizer_utils

---

# training::tokenizer_utils



 [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| | **[load_tokenizer](/python-reference/Namespaces/dd/d78/namespacetraining_1_1tokenizer__utils/#function-load_tokenizer)**(str model_path) |
| str | **[format_chat](/python-reference/Namespaces/dd/d78/namespacetraining_1_1tokenizer__utils/#function-format_chat)**(List] messages[Dict[str, str], tokenizer tokenizer) |

## Detailed Description




```
Shared tokenizer utilities for GNUS-POC training and data preparation.

Provides:
- load_tokenizer: Load a HuggingFace tokenizer from a model path.
- format_chat: Apply the model's chat template to messages.

Centralizing these prevents chat template drift (FOUND-01) — the same template
is used during data preparation and training, ensuring format consistency.
```


## Functions Documentation

### function load_tokenizer

```python
load_tokenizer(
    str model_path
)
```




```
Load a HuggingFace tokenizer from the given model path.

Uses AutoTokenizer.from_pretrained() with trust_remote_code=True
(matching existing convention in train_specialists_mlx.py).
Does NOT require MLX — uses transformers library only.

Args:
    model_path: HuggingFace model ID or local path (e.g.,
                "mlx-community/Qwen3-30B-A3B-Instruct-2507-bf16").

Returns:
    A HuggingFace tokenizer object with apply_chat_template method.

Raises:
    RuntimeError: If tokenizer loading fails.
```


### function format_chat

```python
str format_chat(
    List] messages[Dict[str, str],
    tokenizer tokenizer
)
```




```
Format a list of chat messages using the tokenizer's native chat template.

Calls tokenizer.apply_chat_template() to produce the correct special tokens
for the model (Qwen3, Qwen2.5, etc.). This replaces the hand-rolled
<|im_start|> format that caused the FOUND-01 bug.

Args:
    messages: List of message dicts with 'role' and 'content' keys.
              Example: [{"role": "system", "content": "You are helpful."},
                        {"role": "user", "content": "Hello"}]
    tokenizer: A HuggingFace tokenizer object with apply_chat_template method.

Returns:
    A formatted prompt string using the model's native chat template.

Raises:
    AssertionError: If the returned string is empty.
```






-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700