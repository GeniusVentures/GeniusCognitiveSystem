---
title: sgns::neoswarm::network::ResultAggregation::Config

---

# sgns::neoswarm::network::ResultAggregation::Config






`#include <result_aggregation.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::chrono::milliseconds | **[m_timeout](/source-reference/Classes/d4/daf/structsgns_1_1neoswarm_1_1network_1_1_result_aggregation_1_1_config/#variable-m-timeout)** <br/>max wait for responses  |
| size_t | **[min_responses_](/source-reference/Classes/d4/daf/structsgns_1_1neoswarm_1_1network_1_1_result_aggregation_1_1_config/#variable-min-responses-)** <br/>minimum before returning  |
| size_t | **[max_responses_](/source-reference/Classes/d4/daf/structsgns_1_1neoswarm_1_1network_1_1_result_aggregation_1_1_config/#variable-max-responses-)** <br/>stop collecting after this many  |

## Public Attributes Documentation

### variable m_timeout

```cpp
std::chrono::milliseconds m_timeout { 5000 };
```

max wait for responses 

### variable min_responses_

```cpp
size_t min_responses_ = 1;
```

minimum before returning 

### variable max_responses_

```cpp
size_t max_responses_ = 10;
```

stop collecting after this many 

-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700