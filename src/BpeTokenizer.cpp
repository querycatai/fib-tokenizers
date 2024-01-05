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

    auto it = vocab_map_.find(unk_token);
    if (it != end(vocab_map_))
        unk_token_id_ = it->second;

    for (auto& p : vocab_map_)
        vocab_index_map_.emplace(p.second, p.first);

    for (int32_t i = 33; i <= 126; ++i)
        byte_encoder_[i] = GetEncoding(utf8String((char32_t)i));

    for (int32_t i = 161; i <= 172; ++i)
        byte_encoder_[i] = GetEncoding(utf8String((char32_t)i));

    for (int32_t i = 174; i <= 255; ++i)
        byte_encoder_[i] = GetEncoding(utf8String((char32_t)i));

    int32_t index = 256;
    for (int32_t i = 0; i < 33; ++i)
        byte_encoder_[i] = GetEncoding(utf8String((char32_t)(index++)));

    for (int32_t i = 127; i < 161; ++i)
        byte_encoder_[i] = GetEncoding(utf8String((char32_t)(index++)));

    byte_encoder_[173] = GetEncoding(utf8String((char32_t)(index++)));

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
        for (char& cp : tok)
            byte_list.push_back(std::make_pair(byte_encoder_[static_cast<unsigned char>(cp)], 1));

        bpe(byte_list);

        for (auto p : byte_list)
            push_back(p.first, p.second);
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
}
