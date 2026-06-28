---
title: sgns::neoswarm::PromptFeatures

---

# sgns::neoswarm::PromptFeatures






`#include <types.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| float | **[numeric_density_](/source-reference/Classes/d4/dc5/structsgns_1_1neoswarm_1_1_prompt_features/#variable-numeric-density-)** <br/>ratio of numeric tokens  |
| bool | **[has_code_syntax_](/source-reference/Classes/d4/dc5/structsgns_1_1neoswarm_1_1_prompt_features/#variable-has-code-syntax-)**  |
| float | **[complexity_](/source-reference/Classes/d4/dc5/structsgns_1_1neoswarm_1_1_prompt_features/#variable-complexity-)** <br/>token count / vocab diversity  |
| size_t | **[token_count_](/source-reference/Classes/d4/dc5/structsgns_1_1neoswarm_1_1_prompt_features/#variable-token-count-)**  |
| bool | **[has_math_keywords_](/source-reference/Classes/d4/dc5/structsgns_1_1neoswarm_1_1_prompt_features/#variable-has-math-keywords-)**  |
| bool | **[has_grammar_request_](/source-reference/Classes/d4/dc5/structsgns_1_1neoswarm_1_1_prompt_features/#variable-has-grammar-request-)**  |

## Public Attributes Documentation

### variable numeric_density_

```cpp
float numeric_density_ = 0.0f;
```

ratio of numeric tokens 

### variable has_code_syntax_

```cpp
bool has_code_syntax_ = false;
```


### variable complexity_

```cpp
float complexity_ = 0.0f;
```

token count / vocab diversity 

### variable token_count_

```cpp
size_t token_count_ = 0;
```


### variable has_math_keywords_

```cpp
bool has_math_keywords_ = false;
```


### variable has_grammar_request_

```cpp
bool has_grammar_request_ = false;
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700