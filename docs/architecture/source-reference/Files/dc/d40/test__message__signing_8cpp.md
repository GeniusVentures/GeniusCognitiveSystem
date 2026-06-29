---
title: GNUS-NEO-SWARM/test/security/test_message_signing.cpp
summary: Unit tests for MessageSigning — verify, tamper rejection, replay protection. 

---

# GNUS-NEO-SWARM/test/security/test_message_signing.cpp



Unit tests for [MessageSigning](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/) — verify, tamper rejection, replay protection.  [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| | **[TEST](/source-reference/Files/dc/d40/test__message__signing_8cpp/#function-test)**([MessageSigning](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/) , VerifyValidSignature ) |
| | **[TEST](/source-reference/Files/dc/d40/test__message__signing_8cpp/#function-test)**([MessageSigning](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/) , VerifyTamperedPayload ) |
| | **[TEST](/source-reference/Files/dc/d40/test__message__signing_8cpp/#function-test)**([MessageSigning](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/) , VerifyWrongKey ) |
| | **[TEST](/source-reference/Files/dc/d40/test__message__signing_8cpp/#function-test)**([MessageSigning](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/) , VerifyEmptySignature ) |
| | **[TEST](/source-reference/Files/dc/d40/test__message__signing_8cpp/#function-test)**([MessageSigning](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/) , VerifyTruncatedSignature ) |
| | **[TEST](/source-reference/Files/dc/d40/test__message__signing_8cpp/#function-test)**([MessageSigning](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/) , VerifyAndStripValid ) |
| | **[TEST](/source-reference/Files/dc/d40/test__message__signing_8cpp/#function-test)**([MessageSigning](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/) , VerifyAndStripExpiredTimestamp ) |

## Detailed Description

Unit tests for [MessageSigning](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/) — verify, tamper rejection, replay protection. 

**Date**: 2026-05-28 GSD Executor 

## Functions Documentation

### function TEST

```cpp
TEST(
    MessageSigning ,
    VerifyValidSignature 
)
```


### function TEST

```cpp
TEST(
    MessageSigning ,
    VerifyTamperedPayload 
)
```


### function TEST

```cpp
TEST(
    MessageSigning ,
    VerifyWrongKey 
)
```


### function TEST

```cpp
TEST(
    MessageSigning ,
    VerifyEmptySignature 
)
```


### function TEST

```cpp
TEST(
    MessageSigning ,
    VerifyTruncatedSignature 
)
```


### function TEST

```cpp
TEST(
    MessageSigning ,
    VerifyAndStripValid 
)
```


### function TEST

```cpp
TEST(
    MessageSigning ,
    VerifyAndStripExpiredTimestamp 
)
```




## Source code

```cpp


#include "security/message_signing.hpp"
#include "security/node_identity.hpp"
#include <gtest/gtest.h>

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

using namespace sgns::neoswarm;
using namespace sgns::neoswarm::security;

namespace
{
    std::string PubKeyToHex( const NodeIdentity::PubKey& key )
    {
        std::ostringstream oss;
        for ( auto b : key )
        {
            oss << std::hex << std::setw( 2 ) << std::setfill( '0' ) << static_cast<int>( b );
        }
        return oss.str();
    }
} // namespace

// =======================================================================
// Signature Verification
// =======================================================================

TEST( MessageSigning, VerifyValidSignature )
{
    NodeIdentity ident;
    ASSERT_TRUE( ident.Generate().has_value() );

    const std::string pubKeyHex = PubKeyToHex( ident.GetPublicKey() );
    MessageSigning signer( ident );

    const std::string payload = R"({"msg":"hello"})";
    auto sig = signer.Sign( payload );
    ASSERT_TRUE( sig.has_value() );
    ASSERT_FALSE( sig.value().empty() );

    EXPECT_TRUE( MessageSigning::Verify( payload, sig.value(), pubKeyHex ) );
}

TEST( MessageSigning, VerifyTamperedPayload )
{
    NodeIdentity ident;
    ASSERT_TRUE( ident.Generate().has_value() );

    const std::string pubKeyHex = PubKeyToHex( ident.GetPublicKey() );
    MessageSigning signer( ident );

    const std::string payload = R"({"msg":"hello"})";
    auto sig = signer.Sign( payload );
    ASSERT_TRUE( sig.has_value() );

    const std::string tampered = R"({"msg":"world"})";
    EXPECT_FALSE( MessageSigning::Verify( tampered, sig.value(), pubKeyHex ) );
}

TEST( MessageSigning, VerifyWrongKey )
{
    NodeIdentity identA;
    NodeIdentity identB;
    ASSERT_TRUE( identA.Generate().has_value() );
    ASSERT_TRUE( identB.Generate().has_value() );

    const std::string pubKeyB = PubKeyToHex( identB.GetPublicKey() );
    MessageSigning signerA( identA );

    const std::string payload = R"({"msg":"hello"})";
    auto sig = signerA.Sign( payload );
    ASSERT_TRUE( sig.has_value() );

    EXPECT_FALSE( MessageSigning::Verify( payload, sig.value(), pubKeyB ) );
}

TEST( MessageSigning, VerifyEmptySignature )
{
    NodeIdentity ident;
    ASSERT_TRUE( ident.Generate().has_value() );

    const std::string pubKeyHex = PubKeyToHex( ident.GetPublicKey() );

    EXPECT_FALSE( MessageSigning::Verify( "payload", {}, pubKeyHex ) );
}

TEST( MessageSigning, VerifyTruncatedSignature )
{
    NodeIdentity ident;
    ASSERT_TRUE( ident.Generate().has_value() );

    const std::string pubKeyHex = PubKeyToHex( ident.GetPublicKey() );

    std::vector<uint8_t> truncated = { 0x30, 0x06, 0x02, 0x01, 0x01, 0x02, 0x01, 0x01 };
    EXPECT_FALSE( MessageSigning::Verify( "payload", truncated, pubKeyHex ) );
}

// =======================================================================
// Nonce + Timestamp Replay Protection
// =======================================================================

TEST( MessageSigning, VerifyAndStripValid )
{
    NodeIdentity ident;
    ASSERT_TRUE( ident.Generate().has_value() );

    const std::string pubKeyHex = PubKeyToHex( ident.GetPublicKey() );
    MessageSigning signer( ident );

    const std::string original = R"({"msg":"hello"})";
    std::string payload = signer.AttachSignature( original );

    EXPECT_NE( payload, original );
    EXPECT_TRUE( MessageSigning::VerifyAndStrip( payload, pubKeyHex ) );
    EXPECT_EQ( payload, original );
}

TEST( MessageSigning, VerifyAndStripExpiredTimestamp )
{
    NodeIdentity ident;
    ASSERT_TRUE( ident.Generate().has_value() );

    const std::string pubKeyHex = PubKeyToHex( ident.GetPublicKey() );
    MessageSigning signer( ident );

    std::string payload = signer.AttachSignature( R"({"msg":"test"})" );

    auto tsPos = payload.rfind( ",\"ts\":" );
    ASSERT_NE( tsPos, std::string::npos );

    uint64_t oldTs = MessageSigning::CurrentTimestampMs() - 61000;
    auto tsVal = payload.find_first_of( "0123456789", tsPos + 6 );
    auto tsEnd = payload.find_first_of( ",}", tsVal );
    ASSERT_NE( tsVal, std::string::npos );
    ASSERT_NE( tsEnd, std::string::npos );

    payload.replace( tsVal, tsEnd - tsVal, std::to_string( oldTs ) );

    EXPECT_FALSE( MessageSigning::VerifyAndStrip( payload, pubKeyHex ) );
}
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
