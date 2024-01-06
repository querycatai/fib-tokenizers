#include "Tokenizer.h"
#include "string_util.h"
#include "unicode.h"

Napi::Value Tokenizer::get_all_special_tokens(const Napi::CallbackInfo& info)
{
    std::vector<std::string> tokens;

    for (auto& token : special_tokens)
        tokens.emplace_back(token.second);

    return to_value(info.Env(), tokens);
}

int32_t Tokenizer::convert_token_to_id(std::string_view token)
{
    auto it = special_tokens.find(token);
    if (it != special_tokens.end())
        return it->second.id;

    return model_token_to_id(token);
}

void Tokenizer::put_token(int32_t token, int32_t index, const std::function<void(int32_t, int32_t)>& push_back)
{
    if (token == model_unk_id)
        push_back(unk_id, index);
    else
        push_back(token + offset, index);
}

void Tokenizer::put_token(const std::string& token, int32_t index, const std::function<void(const std::string&, int32_t)>& push_back)
{
    push_back(token, index);
}

template <typename T>
void Tokenizer::legacy_encode(std::string_view text, std::vector<T>* ids, int32_t prefix_count)
{
    int32_t start = 0;
    std::string temp_string;

    if (do_lower_case) {
        std::u32string text32;

        utf8::convert(text, text32);
        std::transform(text32.begin(), text32.end(), text32.begin(),
            [](char32_t c) { return ufal::unilib::unicode::lowercase(c); });

        utf8::convert(text32, temp_string);
        text = temp_string;
    }

    if (!legacy) {
        std::string temp_string_;
        if (ids->size() > prefix_count)
            temp_string_ = "_";
        else
            temp_string_ = "_ ";

        temp_string_.append(text);
        temp_string = std::move(temp_string_);
        text = temp_string;
        start = 1;
    }

    encode(text, [&](const T& token, int32_t index) {
        if (index >= start)
            ids->emplace_back(token);
    });
}

template <typename T>
void Tokenizer::encode(std::string& text, std::vector<T>* ids)
{
    size_t lastPos = 0;
    int32_t prefix_count = ids->size();

    if (add_prefix_space && text.length() > 0 && !IsSpace(text[0]))
        text = " " + text;

    if (has_pattern) {
        std::smatch m;
        std::string::const_iterator searchStart(text.cbegin());

        while (std::regex_search(searchStart, text.cend(), m, pattern)) {
            auto it = special_tokens.find(std::string_view(&*m[2].first, m[2].length()));
            if (it == special_tokens.end()) {
                searchStart = m[2].first + 1;
                continue;
            }

            const SpecialToken& token = it->second;
            if (token.single_word) {
                if ((m[0].first == m[2].first && m[0].first != text.cbegin())
                    || (m[0].second == m[2].second && m[0].second != text.cend())) {
                    searchStart = m[2].first + 1;
                    continue;
                }
            }

            size_t pos = (token.lstrip ? m[0].first : m[2].first) - text.cbegin();
            if (pos != lastPos)
                legacy_encode(std::string_view(text.data() + lastPos, pos - lastPos), ids, prefix_count);

            ids->emplace_back(token);

            searchStart = token.rstrip ? m[0].first + m[0].length() : m[2].first + m[2].length();
            lastPos = searchStart - text.cbegin();
        }
    }

    if (lastPos < text.size())
        legacy_encode(std::string_view(text.data() + lastPos, text.size() - lastPos), ids, prefix_count);
}

Napi::Value Tokenizer::tokenize(const Napi::CallbackInfo& info)
{
    std::string text = from_value<std::string>(info[0]);
    std::vector<std::string> tokens;

    encode(text, &tokens);

    return to_value(info.Env(), tokens);
}

Napi::Value Tokenizer::encode(const Napi::CallbackInfo& info)
{
    std::string text = from_value<std::string>(info[0]);
    std::vector<int32_t> ids;

    for (auto& token : prefix_tokens)
        ids.emplace_back(token);

    encode(text, &ids);

    if (add_eos_if_not_present && add_eos_token && ids.size() > 0 && ids[ids.size() - 1] == eos_id)
        for (int32_t i = 1; i < suffix_tokens.size(); i++)
            ids.emplace_back(suffix_tokens[i]);
    else
        for (auto& token : suffix_tokens)
            ids.emplace_back(token);

    return to_value(info.Env(), ids);
}

Napi::Value Tokenizer::decode(const Napi::CallbackInfo& info)
{
    std::vector<int32_t> ids = to_array<int32_t>(info[0]);
    std::string text;
    int32_t pos = 0;

    for (int32_t i = 0; i < ids.size(); i++) {
        int32_t id = ids[i];

        if (id != unk_id && id >= offset && id < vocab_size + offset
            && id_to_token.find(id) == id_to_token.end())
            ids[pos++] = id - offset;
    }
    ids.resize(pos);

    decode(ids, text);

    return to_value(info.Env(), text);
}
