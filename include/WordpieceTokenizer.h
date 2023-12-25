#pragma once

#include "napi_value.h"
#include <node_api.h>
#include <memory>
#include "common.h"

class JSWordpieceTokenizer : public NodeClass<JSWordpieceTokenizer> {
public:
    JSWordpieceTokenizer(napi_env env, napi_callback_info info);

public:
    static napi_value Init(napi_env env);

private:
    static napi_value tokenize(napi_env env, napi_callback_info info);

private:
    std::vector<std::u16string> vocab_array;
    std::unordered_map<std::u16string, int32_t> vocab_;
    std::u16string unk_token_;
    uint32_t max_input_chars_per_word_ = 100;

public:
    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;
};