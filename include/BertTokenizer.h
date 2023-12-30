#pragma once

#include "napi_value.h"
#include <node_api.h>
#include <memory>
#include <list>

class JSBertTokenizer : public Napi::ObjectWrap<JSBertTokenizer> {
private:
    using OffsetMappingType = std::list<std::pair<size_t, size_t>>;

public:
    JSBertTokenizer(const Napi::CallbackInfo& info);

public:
    static Napi::Function Init(Napi::Env env);

private:
    Napi::Value tokenize(const Napi::CallbackInfo& info);
    Napi::Value encode(const Napi::CallbackInfo& info);
    Napi::Value decode(const Napi::CallbackInfo& info);

private:
    bool FindToken(const std::u32string& token)
    {
        return vocab_.find(token) != vocab_.end();
    }

    bool FindTokenId(const std::u32string& token, int32_t& token_id);
    void GreedySearch(const std::u32string& token, std::vector<std::u32string>& tokenized_result);
    bool RemoveTokenizeSpace(int64_t pre_token_id, int64_t new_token_id);

private:
    std::vector<std::u32string> basic_tokenize(std::u32string& text);
    std::vector<std::u32string> wordpiece_tokenize(std::u32string& text);

private:
    std::vector<std::u32string> vocab_array;
    std::unordered_map<std::u32string, int32_t> vocab_;
    std::vector<bool> is_substr_;

private:
    bool do_basic_tokenize_;
    bool do_lower_case_;
    bool tokenize_chinese_chars_;
    bool strip_accents_;
    std::u32string suffix_indicator_;
    int64_t max_input_chars_per_word_ = 100;

    std::u32string unk_token_;
    int32_t unk_token_id_ = 0;

    std::u32string sep_token_;
    int32_t sep_token_id_ = 0;

    std::u32string pad_token_;
    int32_t pad_token_id_ = 0;

    std::u32string cls_token_;
    int32_t cls_token_id_ = 0;

    std::u32string mask_token_;
    int32_t mask_token_id_ = 0;

    bool tokenize_punctuation_ = true;
    bool remove_control_chars_ = true;
};