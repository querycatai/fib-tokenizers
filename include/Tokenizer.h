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
    Napi::Value get_all_special_tokens(const Napi::CallbackInfo& info)                      \
    {                                                                                       \
        return Tokenizer::get_all_special_tokens(info);                                     \
    }                                                                                       \
    Napi::Value tokenize(const Napi::CallbackInfo& info)                                    \
    {                                                                                       \
        return Tokenizer::tokenize(info);                                                   \
    }                                                                                       \
    Napi::Value encode(const Napi::CallbackInfo& info)                                      \
    {                                                                                       \
        return Tokenizer::encode(info);                                                     \
    }                                                                                       \
    Napi::Value decode(const Napi::CallbackInfo& info)                                      \
    {                                                                                       \
        return Tokenizer::decode(info);                                                     \
    }

class Tokenizer {
public:
    Tokenizer(bool add_basic_tokens_ = true)
        : add_basic_tokens(add_basic_tokens_)
    {
    }

public:
    void init(Napi::Config opt, int32_t vocab_size_, int32_t unk_id_);

protected:
    Napi::Value get_all_special_tokens(const Napi::CallbackInfo& info);
    Napi::Value tokenize(const Napi::CallbackInfo& info);
    Napi::Value encode(const Napi::CallbackInfo& info);
    Napi::Value decode(const Napi::CallbackInfo& info);

private:
    virtual int32_t model_token_to_id(std::string_view token) = 0;
    virtual void encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back) = 0;
    virtual void encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back) = 0;
    virtual void decode(const std::vector<int32_t>& ids, std::string& text) = 0;

protected:
    int32_t convert_token_to_id(std::string_view token);
    void put_token(int32_t token, int32_t index, const std::function<void(int32_t, int32_t)>& push_back);
    void put_token(const std::string& token, int32_t index, const std::function<void(const std::string&, int32_t)>& push_back);

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
    void config_pattern(const Napi::Config& opt);

private:
    bool legacy = true;
    int32_t vocab_size = 0;
    int32_t offset = 0;
    int32_t special_token_offset;

    bool add_basic_tokens = false;
    int32_t model_unk_id;
    int32_t unk_id = 0;
    int32_t bos_id = 0;
    int32_t eos_id = 0;
    int32_t pad_id = 0;

    bool add_prefix_space = false;
    bool do_lower_case = false;

    std::unordered_map<std::string_view, SpecialToken> special_tokens;
    std::unordered_map<int32_t, std::string> id_to_token;

    bool add_bos_token = false;
    bool add_eos_token = false;
    bool add_eos_if_not_present = false;

    std::vector<int32_t> prefix_tokens;
    std::vector<int32_t> suffix_tokens;

    std::regex pattern;
    bool has_pattern = false;
};
