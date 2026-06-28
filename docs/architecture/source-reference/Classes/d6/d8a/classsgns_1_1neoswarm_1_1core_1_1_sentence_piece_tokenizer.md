---
title: sgns::neoswarm::core::SentencePieceTokenizer
summary: SentencePiece tokenizer. 

---

# sgns::neoswarm::core::SentencePieceTokenizer



SentencePiece tokenizer.  [More...](#detailed-description)


`#include <tokenizer.hpp>`

Inherits from [sgns::neoswarm::core::Tokenizer](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/)

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Impl](/source-reference/Classes/d1/d1d/structsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer_1_1_impl/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[SentencePieceTokenizer](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-sentencepiecetokenizer)**(int eos_id =2, int bos_id =1) |
| | **[~SentencePieceTokenizer](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-~sentencepiecetokenizer)**() override |
| outcome::result< void > | **[Load](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-load)**(const std::string & model_path)<br/>Load a SentencePiece .model file.  |
| virtual outcome::result< std::vector< int > > | **[Encode](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-encode)**(const std::string & text) const override<br/>Encode text to token IDs.  |
| virtual outcome::result< std::string > | **[Decode](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-decode)**(const std::vector< int > & ids) const override<br/>Decode token IDs to text.  |
| virtual bool | **[IsEOS](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-iseos)**(int token_id) const override<br/>Check whether a token ID is the end-of-sequence token.  |
| virtual int | **[EosTokenId](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-eostokenid)**() const override |
| virtual int | **[BosTokenId](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-bostokenid)**() const override |
| virtual size_t | **[VocabSize](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/#function-vocabsize)**() const override |

## Additional inherited members

**Public Functions inherited from [sgns::neoswarm::core::Tokenizer](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/)**

|                | Name           |
| -------------- | -------------- |
| virtual | **[~Tokenizer](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-~tokenizer)**() =default |


## Detailed Description

```cpp
class sgns::neoswarm::core::SentencePieceTokenizer;
```

SentencePiece tokenizer. 

Wraps the sentencepiece library when available. Falls back to a simple whitespace tokenizer when not compiled in. 

## Public Functions Documentation

### function SentencePieceTokenizer

```cpp
explicit SentencePieceTokenizer(
    int eos_id =2,
    int bos_id =1
)
```


### function ~SentencePieceTokenizer

```cpp
~SentencePieceTokenizer() override
```


### function Load

```cpp
outcome::result< void > Load(
    const std::string & model_path
)
```

Load a SentencePiece .model file. 

**Parameters**: 

  * **model_path** Path to the .model file. 


**Return**: outcome::success or TokenizerFailed. 

### function Encode

```cpp
virtual outcome::result< std::vector< int > > Encode(
    const std::string & text
) const override
```

Encode text to token IDs. 

**Parameters**: 

  * **text** Input string. 


**Return**: Token ID vector or TokenizerFailed. 

**Reimplements**: [sgns::neoswarm::core::Tokenizer::Encode](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-encode)


### function Decode

```cpp
virtual outcome::result< std::string > Decode(
    const std::vector< int > & ids
) const override
```

Decode token IDs to text. 

**Parameters**: 

  * **ids** Token ID vector. 


**Return**: Decoded string or TokenizerFailed. 

**Reimplements**: [sgns::neoswarm::core::Tokenizer::Decode](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-decode)


### function IsEOS

```cpp
inline virtual bool IsEOS(
    int token_id
) const override
```

Check whether a token ID is the end-of-sequence token. 

**Parameters**: 

  * **token_id** Token ID to check. 


**Return**: True if this is the EOS token. 

**Reimplements**: [sgns::neoswarm::core::Tokenizer::IsEOS](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-iseos)


### function EosTokenId

```cpp
inline virtual int EosTokenId() const override
```


**Return**: The EOS token ID. 

**Reimplements**: [sgns::neoswarm::core::Tokenizer::EosTokenId](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-eostokenid)


### function BosTokenId

```cpp
inline virtual int BosTokenId() const override
```


**Return**: The BOS token ID. 

**Reimplements**: [sgns::neoswarm::core::Tokenizer::BosTokenId](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-bostokenid)


### function VocabSize

```cpp
virtual size_t VocabSize() const override
```


**Return**: The vocabulary size. 

**Reimplements**: [sgns::neoswarm::core::Tokenizer::VocabSize](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/#function-vocabsize)


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700