#pragma once

#include <regex>
#include <string_view>
#include "napi_value.h"
#include "sentencepiece_processor.h"
#include "SpecialToken.h"

template <>
inline SpecialToken from_value<SpecialToken>(const Napi::Value& value)
{
    return SpecialToken(value);
}
class JSSentencepieceTokenizer : public Napi::ObjectWrap<JSSentencepieceTokenizer> {
public:
    JSSentencepieceTokenizer(const Napi::CallbackInfo& info);

public:
    static Napi::Function Init(Napi::Env env);

private:
    Napi::Value get_all_special_tokens(const Napi::CallbackInfo& info);
    Napi::Value tokenize(const Napi::CallbackInfo& info);
    Napi::Value encode(const Napi::CallbackInfo& info);
    Napi::Value decode(const Napi::CallbackInfo& info);

private:
    int convert_token_to_id(std::string_view token);
    void add_token(SpecialToken& token, bool is_unk = false);

    template <typename T>
    void encode(std::string& text, std::vector<T>* ids);

    template <typename T>
    void legacy_encode(std::string_view text, std::vector<T>* ids, int32_t prefix_count);

private:
    void config_tokens_decoder(const Napi::Config& opt);
    void config_basic_tokens(const Napi::Config& opt);
    void config_special_tokens(const Napi::Config& opt);
    void config_added_tokens(const Napi::Config& opt);
    void config_prefix_suffix(const Napi::Config& opt);
    void config_pattern(const Napi::Config& opt);

private:
    void put_token(int token, int32_t index, const std::function<void(int, int)>& push_back);
    void put_token(const std::string& token, int32_t index, const std::function<void(const std::string&, int)>& push_back);

    void encode(std::string_view text, const std::function<void(int, int)>& push_back);
    void encode(std::string_view text, const std::function<void(const std::string&, int)>& push_back);

private:
    sentencepiece::SentencePieceProcessor sentence_piece_;

    int32_t offset = 0;
    int32_t special_token_offset;

    int model_unk_id;
    int unk_id = 0;
    int bos_id = 0;
    int eos_id = 0;
    int pad_id = 0;

    bool legacy;

    std::unordered_map<std::string_view, int> token_to_id;
    std::unordered_map<int, std::string> id_to_token;

    std::unordered_map<std::string, SpecialToken> special_tokens;

    bool add_bos_token;
    bool add_eos_token;
    bool add_eos_if_not_present;

    std::vector<int> prefix_tokens;
    std::vector<int> suffix_tokens;

    std::regex pattern;
    bool has_pattern = false;

public:
    napi_env env_;
    napi_ref wrapper_;
};
