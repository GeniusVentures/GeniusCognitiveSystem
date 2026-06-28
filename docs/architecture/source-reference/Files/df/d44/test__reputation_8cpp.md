---
title: GNUS-NEO-SWARM/test/reputation/test_reputation.cpp
summary: Unit tests for reputation subsystem. 

---

# GNUS-NEO-SWARM/test/reputation/test_reputation.cpp



Unit tests for reputation subsystem.  [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([ReputationScoring](/source-reference/Classes/d1/dd6/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring/) , AccuracyDeltaWithGroundTruth ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([ReputationScoring](/source-reference/Classes/d1/dd6/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring/) , LatencyPenalty ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([ReputationScoring](/source-reference/Classes/d1/dd6/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring/) , ConsistencyBonus ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([ReputationScoring](/source-reference/Classes/d1/dd6/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring/) , ScoreClampedToRange ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([ReputationScoring](/source-reference/Classes/d1/dd6/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring/) , TaskCountIncremented ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([WeightedConsensus](/source-reference/Classes/d7/dc5/classsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus/) , SelectsHighReputationNode ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([WeightedConsensus](/source-reference/Classes/d7/dc5/classsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus/) , SingleNode ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([WeightedConsensus](/source-reference/Classes/d7/dc5/classsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus/) , EmptyInputReturnsDefault ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([WeightedConsensus](/source-reference/Classes/d7/dc5/classsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus/) , BestWeightedScoreStrategy ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([ReputationCRDT](/source-reference/Classes/d2/d29/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_c_r_d_t/) , MergeNewEntry ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([ReputationCRDT](/source-reference/Classes/d2/d29/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_c_r_d_t/) , LWWKeepsLatest ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([ReputationCRDT](/source-reference/Classes/d2/d29/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_c_r_d_t/) , LWWIgnoresOlder ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([ReputationCRDT](/source-reference/Classes/d2/d29/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_c_r_d_t/) , SerializeDeserializeRoundtrip ) |
| std::string | **[UniqueDbPath](/source-reference/Files/df/d44/test__reputation_8cpp/#function-uniquedbpath)**(const std::string & tag) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([ReputationStorage](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/) , PutAndGet ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([ReputationStorage](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/) , GetNotFound ) |
| | **[TEST](/source-reference/Files/df/d44/test__reputation_8cpp/#function-test)**([ReputationStorage](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/) , GetAll ) |

## Detailed Description

Unit tests for reputation subsystem. 

**Date**: 2026-05-08 

## Functions Documentation

### function TEST

```cpp
TEST(
    ReputationScoring ,
    AccuracyDeltaWithGroundTruth 
)
```


### function TEST

```cpp
TEST(
    ReputationScoring ,
    LatencyPenalty 
)
```


### function TEST

```cpp
TEST(
    ReputationScoring ,
    ConsistencyBonus 
)
```


### function TEST

```cpp
TEST(
    ReputationScoring ,
    ScoreClampedToRange 
)
```


### function TEST

```cpp
TEST(
    ReputationScoring ,
    TaskCountIncremented 
)
```


### function TEST

```cpp
TEST(
    WeightedConsensus ,
    SelectsHighReputationNode 
)
```


### function TEST

```cpp
TEST(
    WeightedConsensus ,
    SingleNode 
)
```


### function TEST

```cpp
TEST(
    WeightedConsensus ,
    EmptyInputReturnsDefault 
)
```


### function TEST

```cpp
TEST(
    WeightedConsensus ,
    BestWeightedScoreStrategy 
)
```


### function TEST

```cpp
TEST(
    ReputationCRDT ,
    MergeNewEntry 
)
```


### function TEST

```cpp
TEST(
    ReputationCRDT ,
    LWWKeepsLatest 
)
```


### function TEST

```cpp
TEST(
    ReputationCRDT ,
    LWWIgnoresOlder 
)
```


### function TEST

```cpp
TEST(
    ReputationCRDT ,
    SerializeDeserializeRoundtrip 
)
```


### function UniqueDbPath

```cpp
static std::string UniqueDbPath(
    const std::string & tag
)
```


### function TEST

```cpp
TEST(
    ReputationStorage ,
    PutAndGet 
)
```


### function TEST

```cpp
TEST(
    ReputationStorage ,
    GetNotFound 
)
```


### function TEST

```cpp
TEST(
    ReputationStorage ,
    GetAll 
)
```




## Source code

```cpp


#include "reputation/reputation_crdt.hpp"
#include "reputation/reputation_scoring.hpp"
#include "reputation/reputation_storage.hpp"
#include "reputation/weighted_consensus.hpp"
#include <chrono>
#include <gtest/gtest.h>

using namespace sgns::neoswarm;
using namespace sgns::neoswarm::reputation;

// ---------------------------------------------------------------------------
// ReputationScoring
// ---------------------------------------------------------------------------
TEST( ReputationScoring, AccuracyDeltaWithGroundTruth )
{
    ReputationScoring scoring;
    EXPECT_GT( scoring.DeltaAccuracy( true, 1.0 ), 0.0 );
    EXPECT_LT( scoring.DeltaAccuracy( true, 0.0 ), 0.0 );
}

TEST( ReputationScoring, LatencyPenalty )
{
    ReputationScoring scoring;
    double d1 = scoring.DeltaLatency( 1000.0, 500.0 ); // 2× median
    double d2 = scoring.DeltaLatency( 100.0, 500.0 );  // 0.2× median
    EXPECT_LT( d1, 0.0 );
    EXPECT_GT( d2, d1 );
}

TEST( ReputationScoring, ConsistencyBonus )
{
    ReputationScoring scoring;
    EXPECT_GT( scoring.DeltaConsistency( 1.0f ), scoring.DeltaConsistency( 50.0f ) );
}

TEST( ReputationScoring, ScoreClampedToRange )
{
    ReputationScoring scoring;
    NodeReputation rep;
    rep.m_identityKey = "test-node";
    rep.m_globalScore = 0.99;

    InferenceResponse resp;
    resp.m_output = "correct";
    resp.m_perplexity = 1.0f;
    resp.m_latencyMs = 100.0;
    resp.m_nodeId = "test-node";

    auto updated = scoring.Update( rep, resp, 100.0, std::string( "correct" ), "correct" );
    EXPECT_LE( updated.m_globalScore, 1.0 );
    EXPECT_GE( updated.m_globalScore, 0.0 );
}

TEST( ReputationScoring, TaskCountIncremented )
{
    ReputationScoring scoring;
    NodeReputation rep;
    rep.m_identityKey = "test-node";
    rep.m_taskCount = 5;

    InferenceResponse resp;
    resp.m_output = "answer";
    resp.m_perplexity = 2.0f;
    resp.m_latencyMs = 200.0;
    resp.m_nodeId = "test-node";

    auto updated = scoring.Update( rep, resp, 200.0, std::nullopt, "answer" );
    EXPECT_EQ( updated.m_taskCount, 6u );
}

// ---------------------------------------------------------------------------
// WeightedConsensus
// ---------------------------------------------------------------------------
TEST( WeightedConsensus, SelectsHighReputationNode )
{
    WeightedConsensus consensus;
    std::vector<NodeOutput> outputs = { { "node-A", "815961", 1.0f, 100.0, 0.9 },
                                        { "node-B", "815961", 1.2f, 120.0, 0.7 },
                                        { "node-C", "814000", 2.0f, 150.0, 0.2 } };
    auto winner = consensus.SelectWinner( outputs );
    EXPECT_EQ( winner.m_output, "815961" );
}

TEST( WeightedConsensus, SingleNode )
{
    WeightedConsensus consensus;
    std::vector<NodeOutput> outputs = { { "node-A", "answer", 1.0f, 100.0, 0.8 } };
    EXPECT_EQ( consensus.SelectWinner( outputs ).m_output, "answer" );
}

TEST( WeightedConsensus, EmptyInputReturnsDefault )
{
    WeightedConsensus consensus;
    std::vector<NodeOutput> outputs;
    EXPECT_TRUE( consensus.SelectWinner( outputs ).m_output.empty() );
}

TEST( WeightedConsensus, BestWeightedScoreStrategy )
{
    WeightedConsensus::Config cfg;
    cfg.strategy_ = WeightedConsensus::Strategy::BestWeightedScore;
    WeightedConsensus consensus( cfg );

    std::vector<NodeOutput> outputs = { { "node-A", "wrong", 5.0f, 100.0, 0.9 },
                                        { "node-B", "correct", 1.0f, 100.0, 0.8 } };
    EXPECT_EQ( consensus.SelectWinner( outputs ).m_output, "correct" );
}

// ---------------------------------------------------------------------------
// ReputationCRDT
// ---------------------------------------------------------------------------
TEST( ReputationCRDT, MergeNewEntry )
{
    ReputationCRDT crdt;
    NodeReputation r;
    r.m_identityKey = "node-1";
    r.m_globalScore = 0.8;
    r.m_lastUpdatedMs = 1000;
    crdt.Merge( r );

    auto got = crdt.Get( "node-1" );
    ASSERT_TRUE( got.has_value() );
    EXPECT_DOUBLE_EQ( got->m_globalScore, 0.8 );
}

TEST( ReputationCRDT, LWWKeepsLatest )
{
    ReputationCRDT crdt;
    NodeReputation old_r;
    old_r.m_identityKey = "node-1";
    old_r.m_globalScore = 0.5;
    old_r.m_lastUpdatedMs = 1000;
    crdt.Merge( old_r );

    NodeReputation newer;
    newer.m_identityKey = "node-1";
    newer.m_globalScore = 0.9;
    newer.m_lastUpdatedMs = 2000;
    crdt.Merge( newer );

    EXPECT_DOUBLE_EQ( crdt.Get( "node-1" )->m_globalScore, 0.9 );
}

TEST( ReputationCRDT, LWWIgnoresOlder )
{
    ReputationCRDT crdt;
    NodeReputation newer;
    newer.m_identityKey = "node-1";
    newer.m_globalScore = 0.9;
    newer.m_lastUpdatedMs = 2000;
    crdt.Merge( newer );

    NodeReputation old_r;
    old_r.m_identityKey = "node-1";
    old_r.m_globalScore = 0.3;
    old_r.m_lastUpdatedMs = 500;
    crdt.Merge( old_r );

    EXPECT_DOUBLE_EQ( crdt.Get( "node-1" )->m_globalScore, 0.9 );
}

TEST( ReputationCRDT, SerializeDeserializeRoundtrip )
{
    ReputationCRDT crdt1;
    NodeReputation r;
    r.m_identityKey = "node-X";
    r.m_globalScore = 0.75;
    r.m_taskCount = 42;
    r.m_lastUpdatedMs = 99999;
    crdt1.Merge( r );

    ReputationCRDT crdt2;
    crdt2.DeserializeAndMerge( crdt1.Serialize() );

    auto got = crdt2.Get( "node-X" );
    ASSERT_TRUE( got.has_value() );
    EXPECT_DOUBLE_EQ( got->m_globalScore, 0.75 );
    EXPECT_EQ( got->m_taskCount, 42u );
}

// ---------------------------------------------------------------------------
// ReputationStorage
// ---------------------------------------------------------------------------
static std::string UniqueDbPath( const std::string& tag )
{
    return "/tmp/genius_test_" + tag + "_" +
           std::to_string( std::chrono::steady_clock::now().time_since_epoch().count() );
}

TEST( ReputationStorage, PutAndGet )
{
    ReputationStorage storage( UniqueDbPath( "putget" ) );
    ASSERT_TRUE( storage.Open().has_value() );

    NodeReputation r;
    r.m_identityKey = "test-node";
    r.m_globalScore = 0.65;
    r.m_taskCount = 10;
    ASSERT_TRUE( storage.Put( r ).has_value() );

    auto got = storage.Get( "test-node" );
    ASSERT_TRUE( got.has_value() );
    EXPECT_DOUBLE_EQ( got.value().m_globalScore, 0.65 );
    EXPECT_EQ( got.value().m_taskCount, 10u );
}

TEST( ReputationStorage, GetNotFound )
{
    ReputationStorage storage( UniqueDbPath( "notfound" ) );
    ASSERT_TRUE( storage.Open().has_value() );
    EXPECT_FALSE( storage.Get( "nonexistent" ).has_value() );
}

TEST( ReputationStorage, GetAll )
{
    ReputationStorage storage( UniqueDbPath( "getall" ) );
    ASSERT_TRUE( storage.Open().has_value() );

    for ( int i = 0; i < 5; ++i )
    {
        NodeReputation r;
        r.m_identityKey = "node-" + std::to_string( i );
        r.m_globalScore = 0.5 + i * 0.1;
        ASSERT_TRUE( storage.Put( r ).has_value() );
    }

    auto all = storage.GetAll();
    ASSERT_TRUE( all.has_value() );
    EXPECT_EQ( all.value().size(), 5u );
}
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
