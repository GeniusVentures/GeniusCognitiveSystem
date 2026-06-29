---
title: sgns::neoswarm::core::Tokenizer
summary: Abstract tokenizer interface. 

---

# sgns::neoswarm::core::Tokenizer



Abstract tokenizer interface. 


`#include <tokenizer.hpp>`

Inherited by [sgns::neoswarm::core::SentencePieceTokenizer](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual | **[~Tokenizer](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-~tokenizer)**() =default |
| virtual outcome::result< std::vector< int > > | **[Encode](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-encode)**(const std::string & text) const =0<br/>Encode text to token IDs.  |
| virtual outcome::result< std::string > | **[Decode](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-decode)**(const std::vector< int > & ids) const =0<br/>Decode token IDs to text.  |
| virtual bool | **[IsEOS](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-iseos)**(int token_id) const =0<br/>Check whether a token ID is the end-of-sequence token.  |
| virtual int | **[EosTokenId](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-eostokenid)**() const =0 |
| virtual int | **[BosTokenId](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-bostokenid)**() const =0 |
| virtual size_t | **[VocabSize](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-vocabsize)**() const =0 |

## Public Functions Documentation

### function ~Tokenizer

```cpp
virtual ~Tokenizer() =default
```


### function Encode

```cpp
virtual outcome::result< std::vector< int > > Encode(
    const std::string & text
) const =0
```

Encode text to token IDs. 

**Parameters**: 

  * **text** Input string. 


**Return**: Token ID vector or TokenizerFailed. 

**Reimplemented by**: [sgns::neoswarm::core::SentencePieceTokenizer::Encode](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-encode)


### function Decode

```cpp
virtual outcome::result< std::string > Decode(
    const std::vector< int > & ids
) const =0
```

Decode token IDs to text. 

**Parameters**: 

  * **ids** Token ID vector. 


**Return**: Decoded string or TokenizerFailed. 

**Reimplemented by**: [sgns::neoswarm::core::SentencePieceTokenizer::Decode](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-decode)


### function IsEOS

```cpp
virtual bool IsEOS(
    int token_id
) const =0
```

Check whether a token ID is the end-of-sequence token. 

**Parameters**: 

  * **token_id** Token ID to check. 


**Return**: True if this is the EOS token. 

**Reimplemented by**: [sgns::neoswarm::core::SentencePieceTokenizer::IsEOS](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-iseos)


### function EosTokenId

```cpp
virtual int EosTokenId() const =0
```


**Return**: The EOS token ID. 

**Reimplemented by**: [sgns::neoswarm::core::SentencePieceTokenizer::EosTokenId](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-eostokenid)


### function BosTokenId

```cpp
virtual int BosTokenId() const =0
```


**Return**: The BOS token ID. 

**Reimplemented by**: [sgns::neoswarm::core::SentencePieceTokenizer::BosTokenId](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-bostokenid)


### function VocabSize

```cpp
virtual size_t VocabSize() const =0
```


**Return**: The vocabulary size. 

**Reimplemented by**: [sgns::neoswarm::core::SentencePieceTokenizer::VocabSize](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-vocabsize)


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700