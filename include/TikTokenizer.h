#pragma once

#include "Tokenizer.h"
#include "tik_tokenize.h"

class TikTokenizerCore : public TokenizerCore {
public:
    TikTokenizerCore(std::vector<std::string>& vocab_list, Napi::Config& opt);

private:
    virtual int32_t vocab_size() const;
    virtual int32_t unk_id() const;
    virtual int32_t model_token_to_id(std::string_view token);
    virtual void encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back);
    virtual void encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back);
    virtual void decode(const std::vector<int32_t>& ids, std::string& text);

private:
    std::shared_ptr<GptEncoding> encoder_;
};

class TikTokenizer : public Napi::ObjectWrap<TikTokenizer>,
                     public Tokenizer {
public:
    TikTokenizer(const Napi::CallbackInfo& info);
    DECLARE_CLASS(TikTokenizer);
};
