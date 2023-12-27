#pragma once

#include "napi_value.h"
#include <node_api.h>
#include <memory>

class JSWordpieceTokenizer : public Napi::ObjectWrap<JSWordpieceTokenizer> {
public:
    JSWordpieceTokenizer(const Napi::CallbackInfo& info);

public:
    static Napi::Function Init(Napi::Env env);

private:
    Napi::Value tokenize(const Napi::CallbackInfo& info);

private:
    std::vector<std::u16string> vocab_array;
    std::unordered_map<std::u16string, int32_t> vocab_;
    std::u16string unk_token_;
    uint32_t max_input_chars_per_word_ = 100;
};