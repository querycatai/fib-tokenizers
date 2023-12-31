#pragma once

#include "Tokenizer.h"
#include "SpecialToken.h"

class BertTokenizerCore : public TokenizerCore {
public:
    BertTokenizerCore(std::unordered_map<std::string, int32_t> vocab_map, Napi::Config& opt);

private:
    virtual int32_t vocab_size() const;
    virtual int32_t unk_id() const;
    virtual int32_t model_token_to_id(std::string_view token);
    virtual void encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back);
    virtual void encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back);
    virtual void decode(const std::vector<int32_t>& ids, std::string& text);

private:
    bool FindTokenId(const std::u32string& token, int32_t& token_id);
    void GreedySearch(const std::u32string& token, std::vector<std::u32string>& tokenized_result);
    bool RemoveTokenizeSpace(int64_t pre_token_id, int64_t new_token_id);

private:
    std::vector<std::u32string> basic_tokenize(std::u32string& text);
    std::vector<std::u32string> wordpiece_tokenize(std::u32string& text);

private:
    std::unordered_map<std::u32string, int32_t> vocab_map_;
    std::unordered_map<int32_t, std::u32string> vocab_index_map_;
    std::unordered_map<int32_t, bool> is_substr_;

private:
    bool do_basic_tokenize_;
    bool tokenize_chinese_chars_;
    bool strip_accents_;
    std::u32string suffix_indicator_;
    int64_t max_input_chars_per_word_ = 100;

    std::u32string unk_token_;
    int32_t unk_token_id_ = 0;

    bool tokenize_punctuation_ = true;
    bool remove_control_chars_ = true;
};

class BertTokenizer : public Napi::ObjectWrap<BertTokenizer>,
                      public Tokenizer {
public:
    BertTokenizer(const Napi::CallbackInfo& info);
    DECLARE_CLASS(BertTokenizer);
};