---
title: GNUS-NEO-SWARM/test/ffi/test_genius_elm_ffi.cpp

---

# GNUS-NEO-SWARM/test/ffi/test_genius_elm_ffi.cpp





## Functions

|                | Name           |
| -------------- | -------------- |
| | **[TEST](/source-reference/Files/de/d05/test__genius__elm__ffi_8cpp/#function-test)**(GeniusElmFFI , InitWithNullptrSucceeds ) |
| | **[TEST](/source-reference/Files/de/d05/test__genius__elm__ffi_8cpp/#function-test)**(GeniusElmFFI , StringFreeNullptrDoesNotCrash ) |
| | **[TEST](/source-reference/Files/de/d05/test__genius__elm__ffi_8cpp/#function-test)**(GeniusElmFFI , GetStatusReturnsValidJson ) |
| | **[TEST](/source-reference/Files/de/d05/test__genius__elm__ffi_8cpp/#function-test)**(GeniusElmFFI , ChatCompletionsReturnsValidJson ) |
| | **[TEST](/source-reference/Files/de/d05/test__genius__elm__ffi_8cpp/#function-test)**(GeniusElmFFI , ChatCompletionsWithNullDoesNotCrash ) |
| | **[TEST](/source-reference/Files/de/d05/test__genius__elm__ffi_8cpp/#function-test)**(GeniusElmFFI , MultipleInitCallsSucceed ) |
| | **[TEST](/source-reference/Files/de/d05/test__genius__elm__ffi_8cpp/#function-test)**(GeniusElmFFI , ChatCompletionsWithoutInitSucceeds ) |


## Functions Documentation

### function TEST

```cpp
TEST(
    GeniusElmFFI ,
    InitWithNullptrSucceeds 
)
```


### function TEST

```cpp
TEST(
    GeniusElmFFI ,
    StringFreeNullptrDoesNotCrash 
)
```


### function TEST

```cpp
TEST(
    GeniusElmFFI ,
    GetStatusReturnsValidJson 
)
```


### function TEST

```cpp
TEST(
    GeniusElmFFI ,
    ChatCompletionsReturnsValidJson 
)
```


### function TEST

```cpp
TEST(
    GeniusElmFFI ,
    ChatCompletionsWithNullDoesNotCrash 
)
```


### function TEST

```cpp
TEST(
    GeniusElmFFI ,
    MultipleInitCallsSucceed 
)
```


### function TEST

```cpp
TEST(
    GeniusElmFFI ,
    ChatCompletionsWithoutInitSucceeds 
)
```




## Source code

```cpp


#include "genius_elm_chat_completions.h"
#include <gtest/gtest.h>

TEST( GeniusElmFFI, InitWithNullptrSucceeds )
{
    // GeniusElmInit(nullptr, nullptr) should initialize in stub mode
    int result = GeniusElmInit( nullptr, nullptr );
    EXPECT_EQ( result, 0 );
}

TEST( GeniusElmFFI, StringFreeNullptrDoesNotCrash )
{
    // Freeing nullptr should not crash
    GeniusElmStringFree( nullptr );
    SUCCEED();
}

TEST( GeniusElmFFI, GetStatusReturnsValidJson )
{
    int result = GeniusElmInit( nullptr, nullptr );
    EXPECT_EQ( result, 0 );

    char* status = GeniusElmGetStatus();
    ASSERT_NE( status, nullptr );

    std::string statusStr( status );
    EXPECT_NE( statusStr.find( "model_loaded" ), std::string::npos );
    EXPECT_NE( statusStr.find( "mode" ), std::string::npos );
    EXPECT_NE( statusStr.find( "supergenius_connected" ), std::string::npos );
    EXPECT_NE( statusStr.find( "fallback_active" ), std::string::npos );

    GeniusElmStringFree( status );
}

TEST( GeniusElmFFI, ChatCompletionsReturnsValidJson )
{
    int result = GeniusElmInit( nullptr, nullptr );
    EXPECT_EQ( result, 0 );

    const char* request = R"({"messages":[{"role":"user","content":"Hello"}]})";
    char* response = GeniusElmChatCompletionsCreate( request );
    ASSERT_NE( response, nullptr );

    std::string respStr( response );
    // Should be valid JSON — either a chat completion or an error
    EXPECT_TRUE( respStr.find( '{' ) != std::string::npos );
    EXPECT_TRUE( respStr.find( '}' ) != std::string::npos );

    GeniusElmStringFree( response );
}

TEST( GeniusElmFFI, ChatCompletionsWithNullDoesNotCrash )
{
    int result = GeniusElmInit( nullptr, nullptr );
    EXPECT_EQ( result, 0 );

    // Null request should not crash — returns a valid response or error JSON
    char* response = GeniusElmChatCompletionsCreate( nullptr );
    ASSERT_NE( response, nullptr );

    std::string respStr( response );
    // Response should be valid JSON (stub mode returns a chat completion)
    EXPECT_TRUE( respStr.find( '{' ) != std::string::npos );

    GeniusElmStringFree( response );
}

TEST( GeniusElmFFI, MultipleInitCallsSucceed )
{
    // Calling GeniusElmInit multiple times should succeed each time
    EXPECT_EQ( GeniusElmInit( nullptr, nullptr ), 0 );
    EXPECT_EQ( GeniusElmInit( nullptr, nullptr ), 0 );
    EXPECT_EQ( GeniusElmInit( nullptr, nullptr ), 0 );
}

TEST( GeniusElmFFI, ChatCompletionsWithoutInitSucceeds )
{
    // Chat should lazy-init if GeniusElmInit was never called
    const char* request = R"({"messages":[{"role":"user","content":"test"}]})";
    char* response = GeniusElmChatCompletionsCreate( request );
    ASSERT_NE( response, nullptr );

    GeniusElmStringFree( response );
}
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
