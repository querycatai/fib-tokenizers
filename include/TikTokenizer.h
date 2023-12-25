#pragma once

#include "napi_value.h"
#include "encoding.h"

class JSTikTokenizer : public NodeClass<JSTikTokenizer> {
public:
    JSTikTokenizer(NodeArg<JSTikTokenizer>& args);

public:
    static napi_value Init(napi_env env);

private:
    static napi_value encode(napi_env env, napi_callback_info info);
    static napi_value decode(napi_env env, napi_callback_info info);

private:
    std::shared_ptr<GptEncoding> encoder_;

public:
    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;
};
