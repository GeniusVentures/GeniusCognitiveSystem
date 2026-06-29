---
title: eval::benchmark_mlx_model::MLXBenchmarkModel

---

# eval::benchmark_mlx_model::MLXBenchmarkModel



 [More...](#detailed-description)

Inherits from LM

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#function-__init__)**(self self, Path model_path, Optional adapter_path[Path] =None, int max_length =[kDefaultMaxLength](/python-reference/Namespaces/da/dc4/namespaceeval_1_1benchmark__mlx__model/#variable-kdefaultmaxlength), ** kwargs) |
| List[Tuple[float, bool]] | **[loglikelihood](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#function-loglikelihood)**(self self, requests requests) |
| List[str] | **[generate_until](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#function-generate_until)**(self self, requests requests) |
| List[Tuple[float, bool]] | **[loglikelihood_rolling](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#function-loglikelihood_rolling)**(self self, requests requests) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| None | **[_load_model](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#function-_load_model)**(self self) |
| List[int] | **[_encode](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#function-_encode)**(self self, str text) |
| str | **[_decode](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#function-_decode)**(self self, Sequence token_ids[int]) |
| float | **[_tok_logprob](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#function-_tok_logprob)**(self self, logits logits, int position, int token_id) |
| bool | **[_is_greedy](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#function-_is_greedy)**(self self, logits logits, int position, int token_id) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_model_path](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#variable-_model_path)**  |
| | **[_adapter_path](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#variable-_adapter_path)**  |
| int | **[_batch_size](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#variable-_batch_size)**  |
| | **[_max_length](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#variable-_max_length)**  |
| | **[_model](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#variable-_model)**  |
| | **[_tokenizer](/python-reference/Classes/d8/de9/classeval_1_1benchmark__mlx__model_1_1_m_l_x_benchmark_model/#variable-_tokenizer)**  |

## Detailed Description

```python
class eval::benchmark_mlx_model::MLXBenchmarkModel;
```




```
lm-eval LM wrapper that delegates inference to a local MLX model.

Implements ``loglikelihood()``, ``generate_until()``, and
``loglikelihood_rolling()`` as required by the LM abstract base class.

MLX imports are done defensively inside methods so the module
is importable even in non-MLX test environments.
```

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    Path model_path,
    Optional adapter_path[Path] =None,
    int max_length =kDefaultMaxLength,
    ** kwargs
)
```




```
Initialize the MLX model wrapper and load the model.

Args:
    model_path: Path to the MLX model directory (must exist).
    adapter_path: Optional path to LoRA adapter weights.
    max_length: Maximum sequence length for the model context window.
    **kwargs: Additional arguments passed to ``LM.__init__()``.

Raises:
    FileNotFoundError: If ``model_path`` or ``adapter_path`` do not exist.
    RuntimeError: If MLX model loading fails.
```


### function loglikelihood

```python
List[Tuple[float, bool]] loglikelihood(
    self self,
    requests requests
)
```




```
Compute log-probability of continuation given context.

For each request, encodes ``context + continuation``, forwards
through the model, extracts logprobs at continuation positions,
sums them, and checks whether each continuation token is the
greedy argmax.

Args:
    requests: List of ``Instance`` objects with ``arguments`` set to
        ``(context: str, continuation: str)``.

Returns:
    List of ``(logprob, is_greedy)`` tuples, one per request.

Raises:
    ValueError: If ``requests`` is empty.
    RuntimeError: If MLX inference fails.
```


### function generate_until

```python
List[str] generate_until(
    self self,
    requests requests
)
```




```
Generate text autoregressively until stop conditions are met.

Autoregressive greedy decode for each request. Generation stops
when any stop sequence appears in the decoded text or the maximum
generation token count is reached.

Per T-04-04 mitigation: ``max_gen_toks`` is capped at
``min(requested_max, model_context_window)``.

Args:
    requests: List of ``Instance`` objects with ``arguments`` set to
        ``(context: str, gen_kwargs: dict)`` where ``gen_kwargs["until"]``
        is a string or list of stop sequences and ``gen_kwargs["max_gen_toks"]``
        optionally caps generation length.

Returns:
    List of generated strings (excluding the context), one per request.

Raises:
    ValueError: If ``requests`` is empty.
    RuntimeError: If MLX generation fails.
```


### function loglikelihood_rolling

```python
List[Tuple[float, bool]] loglikelihood_rolling(
    self self,
    requests requests
)
```




```
Compute rolling log-likelihood over full strings.

For each request, encodes the full string and computes the
log-probability of each token given all preceding tokens (sliding
window). Sums the per-token logprobs and checks whether each
token was the greedy argmax.

Args:
    requests: List of ``Instance`` objects with ``arguments`` set to
        ``(text: str,)``.

Returns:
    List of ``(logprob, is_greedy)`` tuples, one per request.

Raises:
    ValueError: If ``requests`` is empty.
    RuntimeError: If MLX inference fails.
```


## Protected Functions Documentation

### function _load_model

```python
None _load_model(
    self self
)
```




```
Load the MLX model and tokenizer.

Uses ``mlx_lm.load()`` to load the model from ``model_path``.
If ``adapter_path`` is provided, loads the LoRA adapter.

Raises:
    RuntimeError: If MLX model loading fails.
```


### function _encode

```python
List[int] _encode(
    self self,
    str text
)
```




```
Encode text to token IDs using the loaded tokenizer.

Handles multiple tokenizer APIs: HuggingFace (encode returns list
or BatchEncoding), and callable tokenizers.

Raises:
    RuntimeError: If no tokenizer is loaded.
```


### function _decode

```python
str _decode(
    self self,
    Sequence token_ids[int]
)
```




```
Decode token IDs to text using the loaded tokenizer.```


### function _tok_logprob

```python
float _tok_logprob(
    self self,
    logits logits,
    int position,
    int token_id
)
```




```
Extract the log-probability of a specific token at a position.

Computes ``log(softmax(logits[0, position]))[token_id]``.

Args:
    logits: Model output tensor of shape (1, seq_len, vocab_size).
    position: Sequence position to extract from.
    token_id: Target token ID to compute logprob for.

Returns:
    Log-probability as a Python float.
```


### function _is_greedy

```python
bool _is_greedy(
    self self,
    logits logits,
    int position,
    int token_id
)
```




```
Check whether *token_id* is the argmax at *position*.

Args:
    logits: Model output tensor of shape (1, seq_len, vocab_size).
    position: Sequence position to check.
    token_id: Token ID to compare against argmax.

Returns:
    True if *token_id* matches the argmax prediction at *position*.
```


## Protected Attributes Documentation

### variable _model_path

```python
_model_path =  model_path;
```


### variable _adapter_path

```python
_adapter_path =  adapter_path;
```


### variable _batch_size

```python
int _batch_size =  1;
```


### variable _max_length

```python
_max_length =  max_length;
```


### variable _model

```python
_model =  None;
```


### variable _tokenizer

```python
_tokenizer =  None;
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700