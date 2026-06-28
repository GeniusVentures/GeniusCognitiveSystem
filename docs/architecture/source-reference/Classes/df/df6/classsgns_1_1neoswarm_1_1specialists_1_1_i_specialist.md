---
title: sgns::neoswarm::specialists::ISpecialist
summary: Abstract interface for specialist post-processing modules. 

---

# sgns::neoswarm::specialists::ISpecialist



Abstract interface for specialist post-processing modules.  [More...](#detailed-description)


`#include <i_specialist.hpp>`

Inherited by [sgns::neoswarm::specialists::GrammarSpecialist](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/), [sgns::neoswarm::specialists::MathSpecialist](/source-reference/Classes/df/d42/classsgns_1_1neoswarm_1_1specialists_1_1_math_specialist/)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual | **[~ISpecialist](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-~ispecialist)**() =default |
| virtual std::string | **[GetName](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-getname)**() const =0 |
| virtual bool | **[IsLoaded](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-isloaded)**() const =0 |
| virtual outcome::result< void > | **[Load](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-load)**(const std::string & model_path) =0<br/>Load the specialist model from disk.  |
| virtual outcome::result< std::string > | **[Process](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-process)**(const std::string & input) =0<br/>Process input (typically Core LLM output) and return refined output.  |
| virtual float | **[GetConfidence](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-getconfidence)**() const =0<br/>Confidence in the last [Process()](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-process) call.  |

## Detailed Description

```cpp
class sgns::neoswarm::specialists::ISpecialist;
```

Abstract interface for specialist post-processing modules. 

Each specialist takes Core LLM output and refines it for a specific domain. 

## Public Functions Documentation

### function ~ISpecialist

```cpp
virtual ~ISpecialist() =default
```


### function GetName

```cpp
virtual std::string GetName() const =0
```


**Return**: Human-readable name of this specialist. 

**Reimplemented by**: [sgns::neoswarm::specialists::GrammarSpecialist::GetName](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/#function-getname), [sgns::neoswarm::specialists::MathSpecialist::GetName](/source-reference/Classes/df/d42/classsgns_1_1neoswarm_1_1specialists_1_1_math_specialist/#function-getname)


### function IsLoaded

```cpp
virtual bool IsLoaded() const =0
```


**Return**: True if the specialist model has been loaded. 

**Reimplemented by**: [sgns::neoswarm::specialists::GrammarSpecialist::IsLoaded](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/#function-isloaded), [sgns::neoswarm::specialists::MathSpecialist::IsLoaded](/source-reference/Classes/df/d42/classsgns_1_1neoswarm_1_1specialists_1_1_math_specialist/#function-isloaded)


### function Load

```cpp
virtual outcome::result< void > Load(
    const std::string & model_path
) =0
```

Load the specialist model from disk. 

**Parameters**: 

  * **model_path** Path to the model file. 


**Return**: outcome::success or ModelLoadFailed. 

**Reimplemented by**: [sgns::neoswarm::specialists::GrammarSpecialist::Load](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/#function-load), [sgns::neoswarm::specialists::MathSpecialist::Load](/source-reference/Classes/df/d42/classsgns_1_1neoswarm_1_1specialists_1_1_math_specialist/#function-load)


### function Process

```cpp
virtual outcome::result< std::string > Process(
    const std::string & input
) =0
```

Process input (typically Core LLM output) and return refined output. 

**Parameters**: 

  * **input** Text to process. 


**Return**: Refined text or InferenceFailed. 

**Reimplemented by**: [sgns::neoswarm::specialists::GrammarSpecialist::Process](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/#function-process), [sgns::neoswarm::specialists::MathSpecialist::Process](/source-reference/Classes/df/d42/classsgns_1_1neoswarm_1_1specialists_1_1_math_specialist/#function-process)


### function GetConfidence

```cpp
virtual float GetConfidence() const =0
```

Confidence in the last [Process()](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-process) call. 

**Return**: Confidence score in [0, 1]. 

**Reimplemented by**: [sgns::neoswarm::specialists::GrammarSpecialist::GetConfidence](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/#function-getconfidence), [sgns::neoswarm::specialists::MathSpecialist::GetConfidence](/source-reference/Classes/df/d42/classsgns_1_1neoswarm_1_1specialists_1_1_math_specialist/#function-getconfidence)


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700