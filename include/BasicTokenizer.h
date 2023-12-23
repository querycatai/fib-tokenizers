#pragma once

#include "napi_value.h"
#include "napi_class.h"
#include <node_api.h>
#include <memory>
#include "common.h"
#include "basic_tokenizer.hpp"

class JSBasicTokenizer : public NodeClass<JSBasicTokenizer> {
public:
    JSBasicTokenizer(napi_env env, napi_callback_info info, napi_value& _this);

public:
    static napi_value Init(napi_env env);

private:
    static napi_value tokenize(napi_env env, napi_callback_info info);

private:
    std::shared_ptr<BasicTokenizer> tokenizer_;

public:
    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;
};