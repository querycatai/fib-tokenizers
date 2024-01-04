#pragma once

#include <regex>
#include <string_view>
#include "napi_value.h"
#include "Tokenizer.h"
#include "sentencepiece_processor.h"

class SentencepieceTokenizer : public Napi::ObjectWrap<SentencepieceTokenizer>,
                               public Tokenizer {
public:
    SentencepieceTokenizer(const Napi::CallbackInfo& info)
        : Napi::ObjectWrap<SentencepieceTokenizer>(info)
    {
        sentence_piece_.LoadFromSerializedProto(from_value<std::string_view>(info[0]));
        Tokenizer::init(info[1], sentence_piece_.GetPieceSize(), sentence_piece_.unk_id());
    }

    DECLARE_CLASS(SentencepieceTokenizer)

private:
    virtual int32_t model_token_to_id(std::string_view token);
    virtual void encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back);
    virtual void encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back);
    virtual void decode(const std::vector<int32_t>& ids, std::string& text);

private:
    sentencepiece::SentencePieceProcessor sentence_piece_;
};
