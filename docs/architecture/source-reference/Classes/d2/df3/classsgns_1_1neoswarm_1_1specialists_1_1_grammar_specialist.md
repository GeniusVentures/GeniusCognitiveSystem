---
title: sgns::neoswarm::specialists::GrammarSpecialist
summary: 200M–500M parameter grammar correction model (PTDS §5.2). 

---

# sgns::neoswarm::specialists::GrammarSpecialist



200M–500M parameter grammar correction model (PTDS §5.2).  [More...](#detailed-description)


`#include <grammar_specialist.hpp>`

Inherits from [sgns::neoswarm::specialists::ISpecialist](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[GrammarSpecialist](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/#function-grammarspecialist)**(std::shared_ptr< [core::InferenceEngine](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/) > engine =nullptr) |
| virtual std::string | **[GetName](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/#function-getname)**() const override |
| virtual bool | **[IsLoaded](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/#function-isloaded)**() const override |
| virtual outcome::result< void > | **[Load](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/#function-load)**(const std::string & model_path) override<br/>Load the specialist model from disk.  |
| virtual outcome::result< std::string > | **[Process](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/#function-process)**(const std::string & input) override<br/>Process input (typically Core LLM output) and return refined output.  |
| virtual float | **[GetConfidence](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/#function-getconfidence)**() const override<br/>Confidence in the last [Process()](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/#function-process) call.  |

## Additional inherited members

**Public Functions inherited from [sgns::neoswarm::specialists::ISpecialist](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/)**

|                | Name           |
| -------------- | -------------- |
| virtual | **[~ISpecialist](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-~ispecialist)**() =default |


## Detailed Description

```cpp
class sgns::neoswarm::specialists::GrammarSpecialist;
```

200M–500M parameter grammar correction model (PTDS §5.2). 

Post-processes Core LLM output for style, consistency, and linguistic correctness. Runs as a sequential stage after Core inference. 

## Public Functions Documentation

### function GrammarSpecialist

```cpp
explicit GrammarSpecialist(
    std::shared_ptr< core::InferenceEngine > engine =nullptr
)
```


### function GetName

```cpp
inline virtual std::string GetName() const override
```


**Return**: Human-readable name of this specialist. 

**Reimplements**: [sgns::neoswarm::specialists::ISpecialist::GetName](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-getname)


### function IsLoaded

```cpp
inline virtual bool IsLoaded() const override
```


**Return**: True if the specialist model has been loaded. 

**Reimplements**: [sgns::neoswarm::specialists::ISpecialist::IsLoaded](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-isloaded)


### function Load

```cpp
virtual outcome::result< void > Load(
    const std::string & model_path
) override
```

Load the specialist model from disk. 

**Parameters**: 

  * **model_path** Path to the model file. 


**Return**: outcome::success or ModelLoadFailed. 

**Reimplements**: [sgns::neoswarm::specialists::ISpecialist::Load](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-load)


### function Process

```cpp
virtual outcome::result< std::string > Process(
    const std::string & input
) override
```

Process input (typically Core LLM output) and return refined output. 

**Parameters**: 

  * **input** Text to process. 


**Return**: Refined text or InferenceFailed. 

**Reimplements**: [sgns::neoswarm::specialists::ISpecialist::Process](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-process)


### function GetConfidence

```cpp
inline virtual float GetConfidence() const override
```

Confidence in the last [Process()](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/#function-process) call. 

**Return**: Confidence score in [0, 1]. 

**Reimplements**: [sgns::neoswarm::specialists::ISpecialist::GetConfidence](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/#function-getconfidence)


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700