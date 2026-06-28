---
title: sgns::neoswarm::specialists::SymbolicFallback
summary: Evaluates mathematical expressions symbolically. 

---

# sgns::neoswarm::specialists::SymbolicFallback



Evaluates mathematical expressions symbolically.  [More...](#detailed-description)


`#include <symbolic_fallback.hpp>`

## Public Functions

|                | Name           |
| -------------- | -------------- |
| std::optional< double > | **[Evaluate](/source-reference/Classes/d1/d06/classsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback/#function-evaluate)**(const std::string & expr)<br/>Evaluate a mathematical expression string.  |
| std::optional< double > | **[ExtractAndEvaluate](/source-reference/Classes/d1/d06/classsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback/#function-extractandevaluate)**(const std::string & text)<br/>Extract the first numeric expression from text and evaluate it.  |
| std::string | **[FormatResult](/source-reference/Classes/d1/d06/classsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback/#function-formatresult)**(double value)<br/>Format a double result as a clean string.  |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| float | **[kConfidenceThreshold](/source-reference/Classes/d1/d06/classsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback/#variable-kconfidencethreshold)**  |

## Detailed Description

```cpp
class sgns::neoswarm::specialists::SymbolicFallback;
```

Evaluates mathematical expressions symbolically. 

Triggered when [MathSpecialist](/source-reference/Classes/df/d42/classsgns_1_1neoswarm_1_1specialists_1_1_math_specialist/) model confidence < kConfidenceThreshold. Supports: +, -, *, /, ^, parentheses, sqrt, abs, sin, cos, tan, log, exp. 

## Public Functions Documentation

### function Evaluate

```cpp
static std::optional< double > Evaluate(
    const std::string & expr
)
```

Evaluate a mathematical expression string. 

**Parameters**: 

  * **expr** Expression string (e.g. "847 * 963"). 


**Return**: Result value or std::nullopt if parsing fails. 

### function ExtractAndEvaluate

```cpp
static std::optional< double > ExtractAndEvaluate(
    const std::string & text
)
```

Extract the first numeric expression from text and evaluate it. 

**Parameters**: 

  * **text** Free-form text containing a math expression. 


**Return**: Result value or std::nullopt if no expression found. 

### function FormatResult

```cpp
static std::string FormatResult(
    double value
)
```

Format a double result as a clean string. 

**Parameters**: 

  * **value** Numeric result. 


**Return**: Integer string if value is whole, decimal string otherwise. 

## Public Attributes Documentation

### variable kConfidenceThreshold

```cpp
static float kConfidenceThreshold = 0.6f;
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700