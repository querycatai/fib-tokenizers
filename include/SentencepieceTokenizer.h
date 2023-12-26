#pragma once

#include <regex>
#include <string_view>
#include "napi_value.h"
#include "sentencepiece_processor.h"

class JSSentencepieceTokenizer : public NodeClass<JSSentencepieceTokenizer> {
public:
    JSSentencepieceTokenizer(NodeArg<JSSentencepieceTokenizer>& args);

public:
    static napi_value Init(napi_env env);

private:
    static napi_value tokenize(napi_env env, napi_callback_info info);
    static napi_value encode(napi_env env, napi_callback_info info);
    static napi_value decode(napi_env env, napi_callback_info info);

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
    static napi_ref constructor;
    napi_env env_;
    napi_ref wrapper_;
};
