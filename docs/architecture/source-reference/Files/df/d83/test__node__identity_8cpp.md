---
title: GNUS-NEO-SWARM/test/security/test_node_identity.cpp
summary: Unit tests for NodeIdentity — key generation, sign/verify, encrypted save/load. 

---

# GNUS-NEO-SWARM/test/security/test_node_identity.cpp



Unit tests for [NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) — key generation, sign/verify, encrypted save/load.  [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| | **[TEST](/source-reference/Files/df/d83/test__node__identity_8cpp/#function-test)**([NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) , DeterministicSignature ) |
| | **[TEST](/source-reference/Files/df/d83/test__node__identity_8cpp/#function-test)**([NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) , DifferentMessagesDifferentSignatures ) |
| | **[TEST](/source-reference/Files/df/d83/test__node__identity_8cpp/#function-test)**([NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) , SignAndVerifyRoundtrip ) |
| | **[TEST](/source-reference/Files/df/d83/test__node__identity_8cpp/#function-test)**([NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) , SaveEncryptedLoadEncryptedRoundtrip ) |
| | **[TEST](/source-reference/Files/df/d83/test__node__identity_8cpp/#function-test)**([NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) , LoadEncryptedWrongPassphrase ) |
| | **[TEST](/source-reference/Files/df/d83/test__node__identity_8cpp/#function-test)**([NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) , LoadEncryptedTamperedFile ) |
| | **[TEST](/source-reference/Files/df/d83/test__node__identity_8cpp/#function-test)**([NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) , SaveEncryptedWithoutKey ) |
| | **[TEST](/source-reference/Files/df/d83/test__node__identity_8cpp/#function-test)**([NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) , LoadEncryptedNonexistentFile ) |
| | **[TEST](/source-reference/Files/df/d83/test__node__identity_8cpp/#function-test)**([NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) , SaveEncryptedOverwrite ) |

## Detailed Description

Unit tests for [NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) — key generation, sign/verify, encrypted save/load. 

**Date**: 2026-05-28 GSD Executor 

## Functions Documentation

### function TEST

```cpp
TEST(
    NodeIdentity ,
    DeterministicSignature 
)
```


### function TEST

```cpp
TEST(
    NodeIdentity ,
    DifferentMessagesDifferentSignatures 
)
```


### function TEST

```cpp
TEST(
    NodeIdentity ,
    SignAndVerifyRoundtrip 
)
```


### function TEST

```cpp
TEST(
    NodeIdentity ,
    SaveEncryptedLoadEncryptedRoundtrip 
)
```


### function TEST

```cpp
TEST(
    NodeIdentity ,
    LoadEncryptedWrongPassphrase 
)
```


### function TEST

```cpp
TEST(
    NodeIdentity ,
    LoadEncryptedTamperedFile 
)
```


### function TEST

```cpp
TEST(
    NodeIdentity ,
    SaveEncryptedWithoutKey 
)
```


### function TEST

```cpp
TEST(
    NodeIdentity ,
    LoadEncryptedNonexistentFile 
)
```


### function TEST

```cpp
TEST(
    NodeIdentity ,
    SaveEncryptedOverwrite 
)
```




## Source code

```cpp


#include "security/node_identity.hpp"
#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <vector>

using namespace sgns::neoswarm;
using namespace sgns::neoswarm::security;

namespace
{
    const std::string kTestKeyPath = "/tmp/gnus_test_node.key";
    const std::string kTestPass = "test123";
    const std::string kWrongPass = "wrong456";

    void RemoveTestFile()
    {
        std::remove( kTestKeyPath.c_str() );
    }
} // namespace

// =======================================================================
// Key Generation & Identity
// =======================================================================

TEST( NodeIdentity, DeterministicSignature )
{
    NodeIdentity ident;
    ASSERT_TRUE( ident.Generate().has_value() );
    ASSERT_TRUE( ident.IsLoaded() );

    std::vector<uint8_t> msg1 = { 0x01, 0x02, 0x03, 0x04 };
    std::vector<uint8_t> msg2 = { 0x01, 0x02, 0x03, 0x04 };

    auto sig1 = ident.Sign( msg1 );
    auto sig2 = ident.Sign( msg2 );
    ASSERT_TRUE( sig1.has_value() );
    ASSERT_TRUE( sig2.has_value() );

    EXPECT_EQ( sig1.value().size(), sig2.value().size() );
    EXPECT_EQ( sig1.value(), sig2.value() );
}

TEST( NodeIdentity, DifferentMessagesDifferentSignatures )
{
    NodeIdentity ident;
    ASSERT_TRUE( ident.Generate().has_value() );

    std::vector<uint8_t> msgA = { 0xAA };
    std::vector<uint8_t> msgB = { 0xBB };

    auto sigA = ident.Sign( msgA );
    auto sigB = ident.Sign( msgB );
    ASSERT_TRUE( sigA.has_value() );
    ASSERT_TRUE( sigB.has_value() );

    EXPECT_NE( sigA.value(), sigB.value() );
}

TEST( NodeIdentity, SignAndVerifyRoundtrip )
{
    NodeIdentity ident;
    ASSERT_TRUE( ident.Generate().has_value() );

    std::vector<uint8_t> msg = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    auto sig = ident.Sign( msg );
    ASSERT_TRUE( sig.has_value() );

    EXPECT_TRUE( ident.Verify( msg, sig.value() ) );
}

// =======================================================================
// AES-256-GCM Encrypted Key Storage
// =======================================================================

TEST( NodeIdentity, SaveEncryptedLoadEncryptedRoundtrip )
{
    RemoveTestFile();

    NodeIdentity ident1;
    ASSERT_TRUE( ident1.Generate().has_value() );
    ASSERT_TRUE( ident1.IsLoaded() );

    auto saveResult = ident1.SaveEncrypted( kTestKeyPath, kTestPass );
    ASSERT_TRUE( saveResult.has_value() );

    NodeIdentity ident2;
    auto loadResult = ident2.LoadEncrypted( kTestKeyPath, kTestPass );
    ASSERT_TRUE( loadResult.has_value() );
    ASSERT_TRUE( ident2.IsLoaded() );

    EXPECT_EQ( ident1.GetPeerId(), ident2.GetPeerId() );

    RemoveTestFile();
}

TEST( NodeIdentity, LoadEncryptedWrongPassphrase )
{
    RemoveTestFile();

    NodeIdentity ident1;
    ASSERT_TRUE( ident1.Generate().has_value() );
    ASSERT_TRUE( ident1.SaveEncrypted( kTestKeyPath, kTestPass ).has_value() );

    NodeIdentity ident2;
    auto result = ident2.LoadEncrypted( kTestKeyPath, kWrongPass );

    EXPECT_FALSE( result.has_value() );
    EXPECT_EQ( result.error(), Error::IdentityError );

    RemoveTestFile();
}

TEST( NodeIdentity, LoadEncryptedTamperedFile )
{
    RemoveTestFile();

    NodeIdentity ident1;
    ASSERT_TRUE( ident1.Generate().has_value() );
    ASSERT_TRUE( ident1.SaveEncrypted( kTestKeyPath, kTestPass ).has_value() );

    {
        std::fstream f( kTestKeyPath, std::ios::binary | std::ios::in | std::ios::out );
        ASSERT_TRUE( f.is_open() );
        f.seekp( 48, std::ios::beg );
        char c = 0;
        f.get( c );
        f.seekp( 48, std::ios::beg );
        f.put( static_cast<char>( c ^ 0xFF ) );
        f.close();
    }

    NodeIdentity ident2;
    auto result = ident2.LoadEncrypted( kTestKeyPath, kTestPass );

    EXPECT_FALSE( result.has_value() );
    EXPECT_EQ( result.error(), Error::IdentityError );

    RemoveTestFile();
}

TEST( NodeIdentity, SaveEncryptedWithoutKey )
{
    RemoveTestFile();

    NodeIdentity ident;
    ASSERT_FALSE( ident.IsLoaded() );

    auto result = ident.SaveEncrypted( kTestKeyPath, kTestPass );

    EXPECT_FALSE( result.has_value() );
    EXPECT_EQ( result.error(), Error::IdentityError );

    RemoveTestFile();
}

TEST( NodeIdentity, LoadEncryptedNonexistentFile )
{
    RemoveTestFile();

    NodeIdentity ident;
    auto result = ident.LoadEncrypted( kTestKeyPath, kTestPass );

    EXPECT_FALSE( result.has_value() );
    EXPECT_EQ( result.error(), Error::IdentityError );
}

TEST( NodeIdentity, SaveEncryptedOverwrite )
{
    RemoveTestFile();

    NodeIdentity ident1;
    ASSERT_TRUE( ident1.Generate().has_value() );
    ASSERT_TRUE( ident1.SaveEncrypted( kTestKeyPath, kTestPass ).has_value() );
    ASSERT_TRUE( ident1.SaveEncrypted( kTestKeyPath, kTestPass ).has_value() );

    NodeIdentity ident2;
    ASSERT_TRUE( ident2.LoadEncrypted( kTestKeyPath, kTestPass ).has_value() );
    EXPECT_EQ( ident1.GetPeerId(), ident2.GetPeerId() );

    RemoveTestFile();
}
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
