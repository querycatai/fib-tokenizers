#pragma once

#include <regex>
#include <string_view>
#include "napi_value.h"
#include "SpecialToken.h"

#define DECLARE_CLASS(class_name)                                                           \
public:                                                                                     \
    static Napi::Function Init(Napi::Env env)                                               \
    {                                                                                       \
        return DefineClass(env, #class_name,                                                \
            { InstanceAccessor<&class_name::get_all_special_tokens>("all_special_tokens"),  \
                InstanceMethod("tokenize", &class_name::tokenize, napi_default_jsproperty), \
                InstanceMethod("encode", &class_name::encode, napi_default_jsproperty),     \
                InstanceMethod("decode", &class_name::decode, napi_default_jsproperty) });  \
    }                                                                                       \
                                                                                            \
private:                                                                                    \
    virtual Napi::Value get_all_special_tokens(const Napi::CallbackInfo& info)              \
    {                                                                                       \
        return Tokenizer::get_all_special_tokens(info);                                     \
    }                                                                                       \
    virtual Napi::Value tokenize(const Napi::CallbackInfo& info)                            \
    {                                                                                       \
        return Tokenizer::tokenize(info);                                                   \
    }                                                                                       \
    virtual Napi::Value encode(const Napi::CallbackInfo& info)                              \
    {                                                                                       \
        return Tokenizer::encode(info);                                                     \
    }                                                                                       \
    virtual Napi::Value decode(const Napi::CallbackInfo& info)                              \
    {                                                                                       \
        return Tokenizer::decode(info);                                                     \
    }

class TokenizerCore {
public:
    virtual int32_t vocab_size() const = 0;
    virtual int32_t unk_id() const = 0;
    virtual int32_t model_token_to_id(std::string_view token) = 0;
    virtual void encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back) = 0;
    virtual void encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back) = 0;
    virtual void decode(const std::vector<int32_t>& ids, std::string& text) = 0;
};

class Tokenizer {
public:
    void init(std::shared_ptr<TokenizerCore> tokenizer_, Napi::Config opt, bool add_basic_tokens_ = true);

public:
    virtual Napi::Value get_all_special_tokens(const Napi::CallbackInfo& info);
    virtual Napi::Value tokenize(const Napi::CallbackInfo& info);
    virtual Napi::Value encode(const Napi::CallbackInfo& info);
    virtual Napi::Value decode(const Napi::CallbackInfo& info);

private:
    int32_t model_token_to_id(std::string_view token)
    {
        return tokenizer->model_token_to_id(token);
    }

    void encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back)
    {
        tokenizer->encode(text, push_back);
    }

    void encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back)
    {
        tokenizer->encode(text, push_back);
    }

    void decode(const std::vector<int32_t>& ids, std::string& text)
    {
        tokenizer->decode(ids, text);
    }

protected:
    int32_t convert_token_to_id(std::string_view token);
    void put_token(int32_t token, const std::function<void(int32_t)>& push_back);
    void put_token(const std::string& token, const std::function<void(const std::string&)>& push_back);

private:
    void add_token(SpecialToken& token, bool is_unk = false);

    template <typename T>
    void encode(std::string& text, std::vector<T>* ids);

    template <typename T>
    void legacy_encode(std::string_view text, std::vector<T>* ids, int32_t prefix_count);

private:
    void config_tokens_decoder(const Napi::Config& opt);
    void config_unk_tokens(const Napi::Config& opt);
    void config_basic_tokens(const Napi::Config& opt);
    void config_special_tokens(const Napi::Config& opt);
    void config_added_tokens(const Napi::Config& opt);
    void config_prefix_suffix(const Napi::Config& opt);
    void config_post_processor(const Napi::Config& opt);
    void config_pattern(const Napi::Config& opt);

private:
    std::shared_ptr<TokenizerCore> tokenizer;

private:
    bool legacy = true;
    int32_t offset = 0;

    bool do_lower_case = false;
    bool add_prefix_space = false;
    bool add_eos_if_not_present = false;
    bool add_basic_tokens = false;

private:
    int32_t vocab_size = 0;
    int32_t special_token_offset;

private:
    int32_t model_unk_id;
    int32_t unk_id = 0;
    int32_t bos_id = 0;
    int32_t eos_id = 0;
    int32_t pad_id = 0;

private:
    std::vector<int32_t> prefix_tokens;
    std::vector<int32_t> suffix_tokens;

private:
    std::unordered_map<std::string_view, SpecialToken> special_tokens;
    std::unordered_map<int32_t, std::string> id_to_token;

private:
    std::regex pattern;
    bool has_pattern = false;
};
