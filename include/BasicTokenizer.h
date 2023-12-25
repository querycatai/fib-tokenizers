#pragma once

#include "napi_value.h"
#include <node_api.h>
#include <memory>
#include "common.h"

class JSBasicTokenizer : public NodeClass<JSBasicTokenizer> {
public:
    JSBasicTokenizer(napi_env env, napi_callback_info info);

public:
    static napi_value Init(napi_env env);

private:
    static napi_value tokenize(napi_env env, napi_callback_info info);

private:
    bool do_lower_case_ = true;
    bool strip_accents_ = true;
    bool tokenize_chinese_chars_ = true;
    bool tokenize_punctuation_ = true;
    bool remove_control_chars_ = true;

public:
    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;
};