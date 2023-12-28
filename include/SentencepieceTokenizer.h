#pragma once

#include <regex>
#include <string_view>
#include "napi_value.h"
#include "sentencepiece_processor.h"

class SpecialToken {
public:
    SpecialToken()
    {
    }

    SpecialToken(std::string content_, int id_ = 0)
        : content(content_)
        , id(id_)
    {
    }

    SpecialToken(Napi::Value value)
    {
        if (value.IsString()) {
            content = value.As<Napi::String>();
            return;
        }

        Napi::Config opt(value);

        content = opt.Get("content", std::string());
        lstrip = opt.Get("lstrip", true);
        normalized = opt.Get("normalized", false);
        rstrip = opt.Get("rstrip", true);
        single_word = opt.Get("single_word", false);
        special = opt.Get("special", false);
    }

public:
    std::string content;
    bool lstrip = true;
    bool normalized = false;
    bool rstrip = true;
    bool single_word = false;
    bool special = false;
    int id = 0;
};

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
    Napi::Value tokenize(const Napi::CallbackInfo& info);
    Napi::Value encode(const Napi::CallbackInfo& info);
    Napi::Value decode(const Napi::CallbackInfo& info);

private:
    int convert_token_to_id(std::string_view token);

    template <typename T>
    void encode(std::string& text, std::vector<T>* ids);

    template <typename T>
    void sentencepiece_encode(char* text, size_t size, std::vector<T>* ids);

private:
    void config_tokens_decoder(const Napi::Config& opt);
    void config_basic_tokens(const Napi::Config& opt);
    void config_special_tokens(const Napi::Config& opt);
    void config_added_tokens(const Napi::Config& opt);
    void config_prefix_suffix(const Napi::Config& opt);
    void config_pattern(const Napi::Config& opt);

private:
    void push_token(int token, std::vector<int>* ids);
    void push_token(const SpecialToken& token, std::vector<int>* ids);
    void push_token(const SpecialToken& token, std::vector<std::string_view>* ids);
    void push_token(const sentencepiece::SentencePieceText_SentencePiece& piece, std::vector<int>* ids);
    void push_token(const sentencepiece::SentencePieceText_SentencePiece& piece, std::vector<std::string_view>* ids);

private:
    sentencepiece::SentencePieceProcessor sentence_piece_;
    std::vector<std::string> added_tokens;
    int32_t offset = 0;

    int unk_id;
    int bos_id;
    int eos_id;

    bool legacy;

    std::unordered_map<std::string_view, int> token_to_id;
    std::unordered_map<int, std::string> id_to_token;

    std::unordered_map<std::string, SpecialToken> special_tokens_map;

    bool add_bos_token;
    bool add_eos_token;

    std::vector<int> prefix_tokens;
    std::vector<int> suffix_tokens;

    std::regex pattern;
    bool has_pattern = false;

public:
    napi_env env_;
    napi_ref wrapper_;
};
