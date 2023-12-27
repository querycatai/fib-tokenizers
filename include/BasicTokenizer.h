#pragma once

#include "napi_value.h"
#include <node_api.h>
#include <memory>

class JSBasicTokenizer : public Napi::ObjectWrap<JSBasicTokenizer> {
public:
    JSBasicTokenizer(const Napi::CallbackInfo& info);

public:
    static Napi::Function Init(Napi::Env env);

private:
    Napi::Value tokenize(const Napi::CallbackInfo& info);

private:
    bool do_lower_case_ = true;
    bool strip_accents_ = true;
    bool tokenize_chinese_chars_ = true;
    bool tokenize_punctuation_ = true;
    bool remove_control_chars_ = true;
};