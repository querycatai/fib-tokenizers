#pragma once

#include "Tokenizer.h"
#include <list>
#include "unicode.h"

class TokenWithRegularExp {
public:
    void Set(std::string_view val)
    {
        utf8::convert(val, m_text_str);
        m_text = m_text_str;
    }

    std::pair<bool, std::u32string_view> GetNextToken()
    {
        while (!m_text.empty()) {
            auto res = TryMatch();
            if (res.empty()) {
                m_text = m_text.substr(1);
                continue;
            }
            return { true, res };
        }
        return { false, {} };
    }

private:
    std::u32string_view TryMatch()
    {
        // python pattern:
        // 's|'t|'re|'ve|'m|'ll|'d| ?\p{L}+| ?\p{N}+| ?[^\s\p{L}\p{N}]+|\s+(?!\S)|\s+

        // 's|'t|'re|'ve|'m|'ll|'d|
        // Note: the sequencial of the following if should not be switched, which follows the python regex's syntax
        if ((m_text[0] == U'\'') && (m_text.size() > 1)) {
            if ((m_text[1] == U's') || (m_text[1] == U't') || (m_text[1] == U'm') || (m_text[1] == U'd')) {
                std::u32string_view res = m_text.substr(0, 2);
                m_text = m_text.substr(2);
                return res;
            }

            if (m_text.size() > 2) {
                if (((m_text[1] == U'r') && (m_text[2] == U'e')) || ((m_text[1] == U'v') && (m_text[2] == U'e')) || ((m_text[1] == U'l') && (m_text[2] == U'l'))) {
                    std::u32string_view res = m_text.substr(0, 3);
                    m_text = m_text.substr(3);
                    return res;
                }
            }
        }

        // ?\p{L}+
        if ((m_text[0] == U' ') && (m_text.size() > 1) && (ufal::unilib::unicode::category(m_text[1]) & ufal::unilib::unicode::L)) {
            size_t i = 2;
            for (; i < m_text.size(); ++i) {
                if ((ufal::unilib::unicode::category(m_text[i]) & ufal::unilib::unicode::L) == 0)
                    break;
            }
            std::u32string_view res = m_text.substr(0, i);
            m_text = m_text.substr(i);
            return res;
        }

        if (ufal::unilib::unicode::category(m_text[0]) & ufal::unilib::unicode::L) {
            size_t i = 1;
            for (; i < m_text.size(); ++i) {
                if ((ufal::unilib::unicode::category(m_text[i]) & ufal::unilib::unicode::L) == 0)
                    break;
            }
            std::u32string_view res = m_text.substr(0, i);
            m_text = m_text.substr(i);
            return res;
        }

        // ?\p{N}+
        if ((m_text[0] == U' ') && (m_text.size() > 1) && (ufal::unilib::unicode::category(m_text[1]) & ufal::unilib::unicode::N)) {
            size_t i = 2;
            for (; i < m_text.size(); ++i) {
                if ((ufal::unilib::unicode::category(m_text[i]) & ufal::unilib::unicode::N) == 0)
                    break;
            }
            std::u32string_view res = m_text.substr(0, i);
            m_text = m_text.substr(i);
            return res;
        }

        if (ufal::unilib::unicode::category(m_text[0]) & ufal::unilib::unicode::N) {
            size_t i = 1;
            for (; i < m_text.size(); ++i) {
                if ((ufal::unilib::unicode::category(m_text[i]) & ufal::unilib::unicode::N) == 0)
                    break;
            }
            std::u32string_view res = m_text.substr(0, i);
            m_text = m_text.substr(i);
            return res;
        }

        // ?[^\s\p{L}\p{N}]+
        if ((m_text[0] == U' ') && (m_text.size() > 1) && (NotLNZ(m_text[1]))) {
            size_t i = 2;
            for (; i < m_text.size(); ++i) {
                if (!NotLNZ(m_text[i]))
                    break;
            }
            std::u32string_view res = m_text.substr(0, i);
            m_text = m_text.substr(i);
            return res;
        }

        if (NotLNZ(m_text[0])) {
            size_t i = 1;
            for (; i < m_text.size(); ++i) {
                if (!NotLNZ(m_text[i]))
                    break;
            }
            std::u32string_view res = m_text.substr(0, i);
            m_text = m_text.substr(i);
            return res;
        }

        // \s+(?!\S)|\s+
        if ((m_text.size() >= 1) && (IsZ(m_text[0]))) {
            size_t i = 1;
            for (; i < m_text.size(); ++i) {
                if (!IsZ(m_text[i]))
                    break;
            }
            if ((i > 1) && (i != m_text.size())) //\s+(?!\S)
            {
                i--;
                std::u32string_view res = m_text.substr(0, i);
                m_text = m_text.substr(i);
                return res;
            }
            // \s+
            std::u32string_view res = m_text.substr(0, i);
            m_text = m_text.substr(i);
            return res;
        }

        return std::u32string_view {};
    }

    static bool IsZ(char32_t ch)
    {
        auto category = ufal::unilib::unicode::category(ch);
        return (category & ufal::unilib::unicode::Z) != 0;
    }

    static bool NotLNZ(char32_t ch)
    {
        auto category = ufal::unilib::unicode::category(ch);
        if (category & ufal::unilib::unicode::L)
            return false;
        if (category & ufal::unilib::unicode::N)
            return false;
        if (category & ufal::unilib::unicode::Z)
            return false;
        return true;
    }

private:
    std::u32string m_text_str;
    std::u32string_view m_text;
};

class BpeTokenizerCore : public TokenizerCore {
public:
    BpeTokenizerCore(std::map<std::string, int32_t>& vocab_map,
        std::vector<std::string>& merges, Napi::Config& opt);

private:
    virtual int32_t vocab_size() const;
    virtual int32_t unk_id() const;
    virtual int32_t model_token_to_id(std::string_view token);
    virtual void encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back);
    virtual void encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back);
    virtual void decode(const std::vector<int32_t>& ids, std::string& text);

private:
    void init(std::map<std::string, int32_t>& vocab_map, std::vector<std::string>& merges, Napi::Config& opt);
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
    std::map<char32_t, int32_t> byte_decoder_;
    std::map<std::string, int32_t> vocab_map_;
    std::map<int32_t, std::string> vocab_index_map_;
    std::unordered_map<std::pair<int32_t, int32_t>, BpeNode, hash_pair> bpe_map_;
};

class BpeTokenizer : public Napi::ObjectWrap<BpeTokenizer>,
                     public Tokenizer {
public:
    BpeTokenizer(const Napi::CallbackInfo& info);
    DECLARE_CLASS(BpeTokenizer);
};
