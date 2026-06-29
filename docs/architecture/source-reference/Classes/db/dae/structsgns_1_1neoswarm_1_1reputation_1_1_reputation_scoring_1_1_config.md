---
title: sgns::neoswarm::reputation::ReputationScoring::Config

---

# sgns::neoswarm::reputation::ReputationScoring::Config






`#include <reputation_scoring.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| double | **[alpha_](/source-reference/Classes/db/dae/structsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring_1_1_config/#variable-alpha_)** <br/>accuracy weight  |
| double | **[beta_](/source-reference/Classes/db/dae/structsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring_1_1_config/#variable-beta_)** <br/>consensus agreement weight  |
| double | **[gamma_](/source-reference/Classes/db/dae/structsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring_1_1_config/#variable-gamma_)** <br/>latency penalty  |
| double | **[delta_](/source-reference/Classes/db/dae/structsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring_1_1_config/#variable-delta_)** <br/>consistency bonus  |
| double | **[epsilon_](/source-reference/Classes/db/dae/structsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring_1_1_config/#variable-epsilon_)** <br/>perplexity smoothing  |
| double | **[baseline_accuracy_](/source-reference/Classes/db/dae/structsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring_1_1_config/#variable-baseline_accuracy_)**  |

## Public Attributes Documentation

### variable alpha_

```cpp
double alpha_ = 0.10;
```

accuracy weight 

### variable beta_

```cpp
double beta_ = 0.05;
```

consensus agreement weight 

### variable gamma_

```cpp
double gamma_ = 0.02;
```

latency penalty 

### variable delta_

```cpp
double delta_ = 0.03;
```

consistency bonus 

### variable epsilon_

```cpp
double epsilon_ = 1e-6;
```

perplexity smoothing 

### variable baseline_accuracy_

```cpp
double baseline_accuracy_ = 0.5;
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700