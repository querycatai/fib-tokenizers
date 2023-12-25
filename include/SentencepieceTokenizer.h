#pragma once

#include "napi_value.h"
#include "sentencepiece_processor.h"

class JSSentencepieceTokenizer : public NodeClass<JSSentencepieceTokenizer> {
public:
    JSSentencepieceTokenizer(NodeArg<JSSentencepieceTokenizer>& arg);

public:
    static napi_value Init(napi_env env);

private:
    static napi_value tokenize(napi_env env, napi_callback_info info);
    static napi_value encode(napi_env env, napi_callback_info info);
    static napi_value decode(napi_env env, napi_callback_info info);

private:
    sentencepiece::SentencePieceProcessor sentence_piece_;
    std::vector<std::string> added_tokens;
    int32_t offset = 0;
    uint32_t unk_id = 0;

public:
    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;
};
