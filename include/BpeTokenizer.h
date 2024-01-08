#pragma once

#include "Tokenizer.h"
#include <list>
#include "unicode.h"
#include "pcre.h"

class BpeTokenizerCore : public TokenizerCore {
public:
    BpeTokenizerCore(std::unordered_map<std::string, int32_t>& vocab_map,
        std::vector<std::string>& merges, Napi::Config& opt);

    ~BpeTokenizerCore();

private:
    virtual int32_t vocab_size() const;
    virtual int32_t unk_id() const;
    virtual int32_t model_token_to_id(std::string_view token);
    virtual void encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back);
    virtual void encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back);
    virtual void decode(const std::vector<int32_t>& ids, std::string& text);

private:
    void init(std::unordered_map<std::string, int32_t>& vocab_map, std::vector<std::string>& merges, Napi::Config& opt);
    void bpe(std::list<std::pair<int32_t, int32_t>>& vals) const;
    int32_t GetEncoding(const std::string& key) const;
    int32_t GetVocabIndex(const std::string& str) const;
    void bpe_encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back) const;

private:
    struct BpeNode {
        int32_t id;
        int32_t value;
        int32_t length;
    };

    struct hash_pair {
        template <class T1, class T2>
        size_t operator()(const std::pair<T1, T2>& p) const
        {
            auto hash1 = std::hash<T1> {}(p.first);
            auto hash2 = std::hash<T2> {}(p.second);
            return hash1 ^ (hash2 << 16);
        }
    };

    bool clean_up_spaces = false;

    int32_t unk_token_id_ = 0;

    int32_t byte_encoder_[256] = {};
    int32_t word_encoder_[256] = {};
    std::unordered_map<char32_t, int32_t> byte_decoder_;
    std::unordered_map<std::string, int32_t> vocab_map_;
    std::unordered_map<int32_t, std::string> vocab_index_map_;
    std::unordered_map<std::pair<int32_t, int32_t>, BpeNode, hash_pair> bpe_map_;

private:
    pcre* regex_ = nullptr;
};

class BpeTokenizer : public Napi::ObjectWrap<BpeTokenizer>,
                     public Tokenizer {
public:
    BpeTokenizer(const Napi::CallbackInfo& info);
    DECLARE_CLASS(BpeTokenizer);
};
