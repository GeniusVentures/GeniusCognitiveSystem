---
title: sgns::neoswarm::fp4::FP4Tensor
summary: Packed FP4 tensor: each byte holds two nibbles (high = even index). 

---

# sgns::neoswarm::fp4::FP4Tensor



Packed FP4 tensor: each byte holds two nibbles (high = even index). 


`#include <fp4_codec.hpp>`

## Public Functions

|                | Name           |
| -------------- | -------------- |
| size_t | **[NumMacroblocks](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/#function-nummacroblocks)**() const |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::vector< uint8_t > | **[data_](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/#variable-data-)** <br/>packed nibbles  |
| std::vector< float > | **[scales_](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/#variable-scales-)** <br/>one scale per macroblock  |
| size_t | **[rows_](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/#variable-rows-)**  |
| size_t | **[cols_](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/#variable-cols-)**  |

## Public Functions Documentation

### function NumMacroblocks

```cpp
inline size_t NumMacroblocks() const
```


## Public Attributes Documentation

### variable data_

```cpp
std::vector< uint8_t > data_;
```

packed nibbles 

### variable scales_

```cpp
std::vector< float > scales_;
```

one scale per macroblock 

### variable rows_

```cpp
size_t rows_ = 0;
```


### variable cols_

```cpp
size_t cols_ = 0;
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700