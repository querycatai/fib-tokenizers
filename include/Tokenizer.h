#pragma once

#include <regex>
#include <limits>
#include <boost/regex.hpp>
#include <jinja2cpp/template.h>
#include <string_view>
#include "napi_value.h"
#include "SpecialToken.h"

#define DECLARE_CLASS(class_name)                                                                                        \
public:                                                                                                                  \
    static Napi::Function Init(Napi::Env env)                                                                            \
    {                                                                                                                    \
        return DefineClass(env, #class_name,                                                                             \
            { InstanceAccessor<&class_name::get_all_special_tokens>("all_special_tokens"),                               \
                InstanceMethod("tokenize", &class_name::tokenize, napi_default_jsproperty),                              \
                InstanceMethod("encode", &class_name::encode, napi_default_jsproperty),                                  \
                InstanceMethod("encode_plus", &class_name::encode_plus, napi_default_jsproperty),                        \
                InstanceMethod("decode", &class_name::decode, napi_default_jsproperty),                                  \
                InstanceMethod("convert_tokens_to_ids", &class_name::convert_tokens_to_ids, napi_default_jsproperty) }); \
    }                                                                                                                    \
                                                                                                                         \
private:                                                                                                                 \
    Napi::Value get_all_special_tokens(const Napi::CallbackInfo& info)                                                   \
    {                                                                                                                    \
        return Tokenizer::get_all_special_tokens(info);                                                                  \
    }                                                                                                                    \
    Napi::Value tokenize(const Napi::CallbackInfo& info)                                                                 \
    {                                                                                                                    \
        return Tokenizer::tokenize(info);                                                                                \
    }                                                                                                                    \
    Napi::Value encode(const Napi::CallbackInfo& info)                                                                   \
    {                                                                                                                    \
        return Tokenizer::encode(info);                                                                                  \
    }                                                                                                                    \
    Napi::Value encode_plus(const Napi::CallbackInfo& info)                                                              \
    {                                                                                                                    \
        return Tokenizer::encode_plus(info);                                                                             \
    }                                                                                                                    \
    Napi::Value decode(const Napi::CallbackInfo& info)                                                                   \
    {                                                                                                                    \
        return Tokenizer::decode(info);                                                                                  \
    }                                                                                                                    \
    Napi::Value convert_tokens_to_ids(const Napi::CallbackInfo& info)                                                    \
    {                                                                                                                    \
        return Tokenizer::convert_tokens_to_ids(info);                                                                   \
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
    Napi::Value get_all_special_tokens(const Napi::CallbackInfo& info);
    Napi::Value tokenize(const Napi::CallbackInfo& info);
    Napi::Value encode(const Napi::CallbackInfo& info);
    Napi::Value encode_plus(const Napi::CallbackInfo& info);
    Napi::Value decode(const Napi::CallbackInfo& info);
    Napi::Value convert_tokens_to_ids(const Napi::CallbackInfo& info);

private:
    Napi::Value batch_encode(const Napi::CallbackInfo& info);
    Napi::Value pair_encode(const Napi::CallbackInfo& info);

protected:
    int32_t convert_token_to_id(std::string_view token);
    void put_token(int32_t token, const std::function<void(int32_t)>& push_back);
    void put_token(const std::string& token, const std::function<void(const std::string&)>& push_back);

private:
    void add_token(SpecialToken& token, bool is_unk = false);

    template <typename T>
    void special_encode(std::string& text, std::vector<T>* ids, int32_t max_length = std::numeric_limits<int32_t>::max());

    template <typename T>
    void legacy_encode(std::string_view text, std::vector<T>* ids, int32_t max_length, int32_t prefix_count);

    void encode_one(std::string& text, std::vector<int32_t>& ids, std::vector<int32_t>& types, int32_t max_length);
    void encode_pair(std::vector<std::string>& texts, std::vector<int32_t>& ids, std::vector<int32_t>& types, int32_t max_length);

private:
    void config_tokens_decoder(const Napi::Config& opt);
    void config_unk_tokens(const Napi::Config& opt);
    void config_basic_tokens(const Napi::Config& opt);
    void config_special_tokens(const Napi::Config& opt);
    void config_added_tokens(const Napi::Config& opt);
    void config_prefix_suffix(const Napi::Config& opt);
    void config_post_processor(const Napi::Config& opt);
    void config_pattern(const Napi::Config& opt);
    void config_model_input_names(const Napi::Config& opt);

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
    bool padding_left = false;
    int32_t model_max_length = 1024;

private:
    // model_input_names
    bool token_type_ids = false;
    bool attention_mask = true;
    bool position_ids = false;

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
    std::vector<int32_t> single_prefix_tokens;
    std::vector<int32_t> single_suffix_tokens;

    std::vector<int32_t> pair_prefix_tokens;
    std::vector<int32_t> pair_middle_tokens;
    std::vector<int32_t> pair_suffix_tokens;

private:
    std::set<std::string> special_tokens;
    std::unordered_map<std::string, int32_t> token_to_id;
    std::unordered_map<std::string_view, SpecialToken> special_token_to_id;
    std::unordered_map<int32_t, std::string> special_id_to_token;

private:
    jinja2::Template chat_template;

private:
    boost::regex pattern;
    bool has_pattern = false;
};
