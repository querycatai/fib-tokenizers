#pragma once

#include "napi_value.h"
#include "tik_tokenize.h"

class JSTikTokenizer : public Napi::ObjectWrap<JSTikTokenizer> {
public:
    JSTikTokenizer(const Napi::CallbackInfo& info);

public:
    static Napi::Function Init(Napi::Env env);

private:
    Napi::Value tokenize(const Napi::CallbackInfo& info);
    Napi::Value encode(const Napi::CallbackInfo& info);
    Napi::Value decode(const Napi::CallbackInfo& info);

private:
    std::shared_ptr<GptEncoding> encoder_;
};
