#include "BpeTokenizer.h"
#include "nlohmann/json.hpp"
#include "string_util.h"

void BpeTokenizer::bpe(std::list<std::pair<int32_t, int32_t>>& vals) const
{
    while (vals.size() >= 2) {
        auto pos_it = vals.end();
        int32_t minval = std::numeric_limits<int32_t>::max();
        int32_t ori_id1 = 0, ori_id2 = 0;
        int32_t aim_id = 0;
        int32_t token_length = 0;

        for (auto it = vals.begin(); it != vals.end(); ++it) {
            auto it2 = it;
            ++it2;
            if (it2 == vals.end())
                break;

            auto map_it = bpe_map_.find({ it->first, it2->first });
            if (map_it == bpe_map_.end())
                continue;

            if (minval > map_it->second.value) {
                ori_id1 = it->first;
                ori_id2 = it2->first;
                minval = map_it->second.value;
                pos_it = it;
                aim_id = map_it->second.id;
            }
        }

        if (pos_it == vals.end())
            break;

        token_length = pos_it->second;
        pos_it = vals.erase(pos_it);
        pos_it->first = aim_id;
        pos_it->second = pos_it->second + token_length;

        for (++pos_it; pos_it != vals.end(); ++pos_it) {
            if (pos_it->first != ori_id1)
                continue;

            auto it2 = pos_it;
            ++it2;
            if (it2 == vals.end())
                break;

            if (it2->first != ori_id2)
                continue;

            token_length = pos_it->second;
            pos_it = vals.erase(pos_it);
            pos_it->first = aim_id;
            pos_it->second = pos_it->second + token_length;
        }
    }
}

static std::string utf8String(char32_t ch)
{
    std::string result;

    utf8::convert(&ch, 1, result);
    return result;
}

BpeTokenizer::BpeTokenizer(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<BpeTokenizer>(info)
{
    Napi::Config opt(info[2]);

    std::string_view vocab_data = from_value<std::string_view>(info[0]);
    vocab_map_ = std::move(nlohmann::json::parse(vocab_data).get<std::unordered_map<std::string, int32_t>>());

    SpecialToken stoken("<|endoftext|>");
    stoken = opt.Get("unk_token", stoken);
    unk_token = stoken.content;

    clean_up_spaces = opt.Get("clean_up_spaces", clean_up_spaces);

    auto it = vocab_map_.find(unk_token);
    if (it != end(vocab_map_))
        unk_token_id_ = it->second;

    for (auto& p : vocab_map_)
        vocab_index_map_.emplace(p.second, p.first);

    int32_t index = 256;

    for (int32_t i = 0; i < 256; ++i) {
        if ((i >= 0 && i < 33) || (i >= 127 && i < 161) || (i == 173)) {
            byte_decoder_[(char32_t)index] = i;
            byte_encoder_[i] = GetEncoding(utf8String((char32_t)index++));
        } else {
            byte_decoder_[(char32_t)i] = i;
            byte_encoder_[i] = GetEncoding(utf8String((char32_t)i));
        }

        if (clean_up_spaces) {
            word_encoder_[i] = byte_encoder_[i];

            auto it = vocab_index_map_.find(byte_encoder_[i]);
            if (it != vocab_index_map_.end()) {
                std::string tmp = it->second;
                tmp.append("</w>", 4);
                int32_t id = GetEncoding(tmp);
                if (id != unk_token_id_)
                    word_encoder_[i] = id;
            }
        }
    }

    std::string_view merges_data = from_value<std::string_view>(info[1]);
    std::vector<std::string> merges;
    split_vocab(merges_data, merges);

    index = 0;
    for (auto& line : merges) {
        if (line.empty())
            continue;
        if ((line[0] == '#') && (index == 0))
            continue;
        auto pos = line.find(' ');
        if (pos == std::string::npos)
            continue;

        std::string w1 = line.substr(0, pos);
        std::string w2 = line.substr(pos + 1);

        int32_t token_length = w1.length() + w2.length();
        if (w2.find("</w>") != std::string::npos || w1.find("</w>") != std::string::npos)
            token_length -= 4;

        std::pair<int32_t, int32_t> key { GetEncoding(w1), GetEncoding(w2) };
        BpeNode value { GetEncoding(w1 + w2), index++, token_length };
        bpe_map_.emplace(key, value);
    }

    Tokenizer::init(opt, vocab_map_.size(), unk_token_id_);
}

int32_t BpeTokenizer::GetEncoding(const std::string& key) const
{
    auto it = vocab_map_.find(key);
    if (it != end(vocab_map_)) {
        return it->second;
    } else {
        return unk_token_id_;
    }
}

int32_t BpeTokenizer::model_token_to_id(std::string_view token)
{
    return vocab_map_[std::string(token)];
}

void BpeTokenizer::bpe_encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back) const
{
    TokenWithRegularExp regcmp;
    regcmp.Set(text);

    while (true) {
        auto [b, tok] = regcmp.GetNextToken();
        if (!b)
            break;

        std::list<std::pair<int32_t, int32_t>> byte_list;

        auto start = tok.begin();
        auto end = tok.end();

        if (clean_up_spaces) {
            while (start < end && IsSpace(*(end - 1)))
                end--;

            while (start < end)
                if (IsSpace(*start))
                    start++;
                else {
                    std::string tmp;
                    utf8::convert(start++, 1, tmp);

                    if (start == end) {
                        for (int32_t i = 0; i < tmp.length() - 1; ++i)
                            byte_list.push_back(std::make_pair(byte_encoder_[static_cast<unsigned char>(tmp[i])], 1));
                        byte_list.push_back(std::make_pair(word_encoder_[static_cast<unsigned char>(tmp.back())], 1));
                    } else {
                        for (char& ch : tmp)
                            byte_list.push_back(std::make_pair(byte_encoder_[static_cast<unsigned char>(ch)], 1));
                    }
                }
        } else {
            while (start < end) {
                std::string tmp;
                utf8::convert(start++, 1, tmp);

                for (char& ch : tmp)
                    byte_list.push_back(std::make_pair(byte_encoder_[static_cast<unsigned char>(ch)], 1));
            }
        }

        if (byte_list.size() > 0) {
            bpe(byte_list);

            for (auto p : byte_list)
                push_back(p.first, p.second);
        }
    }
}

void BpeTokenizer::encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back)
{
    int32_t i = 0;

    bpe_encode(text, [&](int32_t id, int32_t length) {
        push_back(id, i++);
    });
}

void BpeTokenizer::encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back)
{
    int32_t i = 0;
    int32_t lastPos = 0;

    bpe_encode(text, [&](int32_t id, int32_t length) {
        push_back(vocab_index_map_[id], i++);
        lastPos += length;
    });
}

void BpeTokenizer::decode(const std::vector<int32_t>& ids, std::string& text)
{
    for (int32_t i = 0; i < ids.size(); i++) {
        std::u32string token32;
        bool has_wordend = false;

        if (clean_up_spaces) {
            const std::string& token = vocab_index_map_[ids[i]];
            if (token.length() > 4 && token.substr(token.length() - 4) == "</w>") {
                utf8::convert(token.c_str(), token.length() - 4, token32);
                has_wordend = true;
            } else
                utf8::convert(token, token32);
        } else
            utf8::convert(vocab_index_map_[ids[i]], token32);

        for (auto ch : token32)
            text += byte_decoder_[ch];

        if (has_wordend && i < ids.size() - 1) {
            char next_char = vocab_index_map_[ids[i + 1]][0];

            if (next_char != '.' && next_char != ',' && next_char != '!' && next_char != '?'
                && next_char != '\'' && next_char != '\"')
                text += ' ';
        }
    }
}
