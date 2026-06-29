---
title: sgns::neoswarm::reputation::ReputationScoring
summary: Implements the PTDS §7.2 reputation update formulas. 

---

# sgns::neoswarm::reputation::ReputationScoring



Implements the PTDS §7.2 reputation update formulas.  [More...](#detailed-description)


`#include <reputation_scoring.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Config](/source-reference/Classes/db/dae/structsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring_1_1_config/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[ReputationScoring](/source-reference/Classes/d1/dd6/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring/#function-reputationscoring)**() |
| | **[ReputationScoring](/source-reference/Classes/d1/dd6/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring/#function-reputationscoring)**([Config](/source-reference/Classes/db/dae/structsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring_1_1_config/) cfg) |
| [NodeReputation](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/) | **[Update](/source-reference/Classes/d1/dd6/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring/#function-update)**(const [NodeReputation](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/) & old, const [InferenceResponse](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/) & response, double median_latency_ms, std::optional< std::string > ground_truth, const std::string & m_consensusoutput) const<br/>Compute an updated reputation after a completed task.  |
| double | **[DeltaAccuracy](/source-reference/Classes/d1/dd6/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring/#function-deltaaccuracy)**(bool has_ground_truth, double accuracy) const<br/>Compute the accuracy delta component.  |
| double | **[DeltaLatency](/source-reference/Classes/d1/dd6/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring/#function-deltalatency)**(double latency_ms, double median_latency_ms) const<br/>Compute the latency delta component.  |
| double | **[DeltaConsistency](/source-reference/Classes/d1/dd6/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring/#function-deltaconsistency)**(float perplexity) const<br/>Compute the consistency delta component from perplexity.  |

## Detailed Description

```cpp
class sgns::neoswarm::reputation::ReputationScoring;
```

Implements the PTDS §7.2 reputation update formulas. 

Δ accuracy = α × (was_correct − 0.5) Δ consensus = β × agreed_with_winning_answer Δ latency = −γ × (my_time / median_time) Δ consistency = δ × (1 / perplexity) 

## Public Functions Documentation

### function ReputationScoring

```cpp
ReputationScoring()
```


### function ReputationScoring

```cpp
explicit ReputationScoring(
    Config cfg
)
```


### function Update

```cpp
NodeReputation Update(
    const NodeReputation & old,
    const InferenceResponse & response,
    double median_latency_ms,
    std::optional< std::string > ground_truth,
    const std::string & m_consensusoutput
) const
```

Compute an updated reputation after a completed task. 

**Parameters**: 

  * **old** Current reputation record. 
  * **response** Inference response from this node. 
  * **median_latency_ms** Median latency across all responding nodes (ms). 
  * **ground_truth** Correct answer if available. 
  * **m_consensusoutput** The winning consensus output string. 


**Return**: Updated [NodeReputation](/source-reference/Classes/df/d86/structsgns_1_1neoswarm_1_1reputation_1_1_node_reputation/). 

### function DeltaAccuracy

```cpp
double DeltaAccuracy(
    bool has_ground_truth,
    double accuracy
) const
```

Compute the accuracy delta component. 

**Parameters**: 

  * **has_ground_truth** Whether a ground truth answer is available. 
  * **accuracy** Accuracy score in [0, 1]. 


**Return**: Accuracy delta. 

### function DeltaLatency

```cpp
double DeltaLatency(
    double latency_ms,
    double median_latency_ms
) const
```

Compute the latency delta component. 

**Parameters**: 

  * **latency_ms** This node's latency in ms. 
  * **median_latency_ms** Median latency across all nodes. 


**Return**: Latency delta (negative = penalty). 

### function DeltaConsistency

```cpp
double DeltaConsistency(
    float perplexity
) const
```

Compute the consistency delta component from perplexity. 

**Parameters**: 

  * **perplexity** Model perplexity (lower = more confident). 


**Return**: Consistency delta. 

-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700