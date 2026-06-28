---
title: sgns::neoswarm::specialists::SymbolicFallback::Parser

---

# sgns::neoswarm::specialists::SymbolicFallback::Parser





## Public Functions

|                | Name           |
| -------------- | -------------- |
| double | **[ParseExpr](/source-reference/Classes/d5/d0d/structsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback_1_1_parser/#function-parseexpr)**() |
| double | **[ParseTerm](/source-reference/Classes/d5/d0d/structsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback_1_1_parser/#function-parseterm)**() |
| double | **[ParseFactor](/source-reference/Classes/d5/d0d/structsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback_1_1_parser/#function-parsefactor)**() |
| double | **[ParsePrimary](/source-reference/Classes/d5/d0d/structsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback_1_1_parser/#function-parseprimary)**() |
| void | **[SkipWhitespace](/source-reference/Classes/d5/d0d/structsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback_1_1_parser/#function-skipwhitespace)**() |
| char | **[Peek](/source-reference/Classes/d5/d0d/structsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback_1_1_parser/#function-peek)**() const |
| char | **[Consume](/source-reference/Classes/d5/d0d/structsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback_1_1_parser/#function-consume)**() |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| const std::string & | **[input_](/source-reference/Classes/d5/d0d/structsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback_1_1_parser/#variable-input-)**  |
| size_t | **[pos_](/source-reference/Classes/d5/d0d/structsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback_1_1_parser/#variable-pos-)**  |

## Public Functions Documentation

### function ParseExpr

```cpp
double ParseExpr()
```


### function ParseTerm

```cpp
double ParseTerm()
```


### function ParseFactor

```cpp
double ParseFactor()
```


### function ParsePrimary

```cpp
double ParsePrimary()
```


### function SkipWhitespace

```cpp
void SkipWhitespace()
```


### function Peek

```cpp
char Peek() const
```


### function Consume

```cpp
char Consume()
```


## Public Attributes Documentation

### variable input_

```cpp
const std::string & input_;
```


### variable pos_

```cpp
size_t pos_ = 0;
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700