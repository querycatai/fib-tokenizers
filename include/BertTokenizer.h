#pragma once

#include "napi_value.h"
#include <node_api.h>
#include <memory>

class JSBertTokenizer : public Napi::ObjectWrap<JSBertTokenizer> {
public:
    JSBertTokenizer(const Napi::CallbackInfo& info);

public:
    static Napi::Function Init(Napi::Env env);

private:
    Napi::Value tokenize(const Napi::CallbackInfo& info);
    void basic_tokenize(std::u32string& text, std::vector<std::u32string>* tokens);
    void wordpiece_tokenize(std::u32string& text, std::vector<std::u32string>* tokens);

private:
    std::vector<std::u32string> vocab_array;
    std::unordered_map<std::u32string, int32_t> vocab_;
    std::u32string unk_token_;

    bool do_basic_tokenize = false;

    bool do_lower_case_ = true;
    bool strip_accents_ = true;
    bool tokenize_chinese_chars_ = true;
    bool tokenize_punctuation_ = true;
    bool remove_control_chars_ = true;

    uint32_t max_input_chars_per_word_ = 100;
};