#pragma once

#include <regex>
#include <string_view>
#include "napi_value.h"
#include "sentencepiece_processor.h"

class JSSentencepieceTokenizer : public Napi::ObjectWrap<JSSentencepieceTokenizer> {
public:
    class SpecialTokens {
    public:
        SpecialTokens()
        {
        }

        SpecialTokens(const char* content_)
            : content(content_)
        {
        }

        SpecialTokens(const SpecialTokens& other)
        {
            content = other.content;
            lstrip = other.lstrip;
            normalized = other.normalized;
            rstrip = other.rstrip;
            single_word = other.single_word;
            special = other.special;
        }

        SpecialTokens(NodeValue value)
        {
            NodeOpt opt(value);

            content = opt.Get("content", std::string());
            lstrip = opt.Get("lstrip", false);
            normalized = opt.Get("normalized", false);
            rstrip = opt.Get("rstrip", false);
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
    };

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

    void push_token(std::string_view token, std::vector<int>* ids);
    void push_token(std::string_view token, std::vector<std::string_view>* ids);

    template <typename T>
    void sentencepiece_encode(char* text, size_t size, std::vector<T>* ids);

private:
    sentencepiece::SentencePieceProcessor sentence_piece_;
    std::vector<std::string> added_tokens;
    int32_t offset = 0;

    bool add_bos_token;
    bool add_eos_token;

    std::string bos_token;
    std::string eos_token;
    std::string unk_token;
    std::string pad_token;

    int bos_id;
    int eos_id;
    int unk_id;
    int pad_id;

    bool legacy;

    std::unordered_map<std::string_view, int> token_to_id;
    std::unordered_map<int, std::string> id_to_token;
    std::regex pattern;
    bool has_pattern = false;

public:
    napi_env env_;
    napi_ref wrapper_;
};
