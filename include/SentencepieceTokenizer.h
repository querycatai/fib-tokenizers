#pragma once

#include "Tokenizer.h"
#include "sentencepiece_processor.h"

class SentencepieceTokenizerCore : public TokenizerCore {
public:
    SentencepieceTokenizerCore(std::string_view vocab_data)
    {
        sentence_piece_.LoadFromSerializedProto(vocab_data);
    }

private:
    virtual int32_t vocab_size() const;
    virtual int32_t unk_id() const;
    virtual int32_t model_token_to_id(std::string_view token);
    virtual void encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back);
    virtual void encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back);
    virtual void decode(const std::vector<int32_t>& ids, std::string& text);

private:
    sentencepiece::SentencePieceProcessor sentence_piece_;
};

class SentencepieceTokenizer : public Napi::ObjectWrap<SentencepieceTokenizer>,
                               public Tokenizer {
public:
    SentencepieceTokenizer(const Napi::CallbackInfo& info)
        : Napi::ObjectWrap<SentencepieceTokenizer>(info)
    {
        std::string_view vocab_data = from_value<std::string_view>(info[0]);
        Tokenizer::init(std::make_shared<SentencepieceTokenizerCore>(vocab_data), info[1]);
    }
    DECLARE_CLASS(SentencepieceTokenizer);
};
