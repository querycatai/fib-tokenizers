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

void Tokenizer::put_token(int32_t token, const std::function<void(int32_t)>& push_back)
{
    if (token == model_unk_id)
        push_back(unk_id);
    else
        push_back(token + offset);
}

void Tokenizer::put_token(const std::string& token, const std::function<void(const std::string&)>& push_back)
{
    push_back(token);
}

template <typename T>
void Tokenizer::legacy_encode(std::string_view text, std::vector<T>* ids, int32_t max_length, int32_t prefix_count)
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
        if (index >= start) {
            put_token(token, [&](const T& token) {
                if (ids->size() < max_length)
                    ids->emplace_back(token);
            });
        }
    });
}

template <typename T>
void Tokenizer::special_encode(std::string& text, std::vector<T>* ids, int32_t max_length)
{
    size_t lastPos = 0;
    int32_t prefix_count = ids->size();

    if (add_prefix_space && text.length() > 0 && !IsSpace(text[0]))
        text = " " + text;

    if (has_pattern) {
        boost::smatch m;
        std::string::const_iterator searchStart(text.cbegin());

        while (boost::regex_search(searchStart, text.cend(), m, pattern)) {
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
                legacy_encode(std::string_view(text.data() + lastPos, pos - lastPos), ids, max_length, prefix_count);

            if (ids->size() < max_length)
                ids->emplace_back(token);

            searchStart = token.rstrip ? m[0].first + m[0].length() : m[2].first + m[2].length();
            lastPos = searchStart - text.cbegin();
        }
    }

    if (lastPos < text.size())
        legacy_encode(std::string_view(text.data() + lastPos, text.size() - lastPos), ids, max_length, prefix_count);
}

Napi::Value Tokenizer::tokenize(const Napi::CallbackInfo& info)
{
    std::string text = from_value<std::string>(info[0]);
    std::vector<std::string> tokens;

    special_encode(text, &tokens);

    return to_value(info.Env(), tokens);
}

void Tokenizer::encode(std::string& text, std::vector<int32_t>& ids, int32_t max_length)
{
    for (auto& token : single_prefix_tokens)
        if (ids.size() < max_length)
            ids.emplace_back(token);

    special_encode(text, &ids, max_length - single_suffix_tokens.size());

    if (add_eos_if_not_present && single_suffix_tokens.size() > 0
        && ids.size() > 0 && ids[ids.size() - 1] == eos_id)
        for (int32_t i = 1; i < single_suffix_tokens.size(); i++) {
            if (ids.size() < max_length)
                ids.emplace_back(single_suffix_tokens[i]);
        }
    else
        for (auto& token : single_suffix_tokens) {
            if (ids.size() < max_length)
                ids.emplace_back(token);
        }
}

Napi::Value Tokenizer::encode(const Napi::CallbackInfo& info)
{
    std::vector<int32_t> ids;

    std::string text = from_value<std::string>(info[0]);

    Napi::Config opt(info[1]);
    int32_t max_length = opt.Get("max_length", std::numeric_limits<int32_t>::max());

    encode(text, ids, max_length);

    return to_value(info.Env(), ids);
}

Napi::Value Tokenizer::batch_encode(const Napi::CallbackInfo& info)
{
    std::vector<std::vector<int32_t>> ids;
    std::vector<std::vector<int32_t>> masks;

    std::vector<std::string> texts = to_array<std::string>(info[0]);

    Napi::Config opt(info[1]);
    bool padding = opt.Get("padding", false);
    bool truncation = opt.Get("truncation", false);
    int32_t max_length = truncation ? opt.Get("max_length", model_max_length) : std::numeric_limits<int32_t>::max();

    int32_t max_line_length = 0;
    for (auto& text : texts) {
        std::vector<int32_t> ids_;
        std::vector<int32_t> masks_;

        encode(text, ids_, max_length);
        if (ids_.size() > max_line_length)
            max_line_length = ids_.size();

        for (int32_t i = 0; i < ids_.size(); i++)
            masks_.emplace_back(1);

        ids.emplace_back(std::move(ids_));
        masks.emplace_back(std::move(masks_));
    }

    if (padding) {
        for (int32_t i = 0; i < ids.size(); i++) {
            std::vector<int32_t>& ids_ = ids[i];
            std::vector<int32_t>& masks_ = masks[i];

            if (ids_.size() < max_line_length) {
                if (padding_left) {
                    for (int32_t j = ids_.size(); j < max_line_length; j++) {
                        ids_.emplace(ids_.begin(), pad_id);
                        masks_.emplace(masks_.begin(), 0);
                    }
                } else {
                    for (int32_t j = ids_.size(); j < max_line_length; j++) {
                        ids_.emplace_back(pad_id);
                        masks_.emplace_back(0);
                    }
                }
            }
        }
    }

    Napi::Object result = Napi::Object::New(info.Env());
    result.Set("input_ids", to_value(info.Env(), ids));
    result.Set("attention_mask", to_value(info.Env(), masks));

    return result;
}

Napi::Value Tokenizer::pair_encode(const Napi::CallbackInfo& info)
{
    std::vector<int32_t> ids;
    std::vector<int32_t> types;
    std::vector<int32_t> masks;

    std::vector<std::string> texts;
    Napi::Config opt;
    int32_t i;

    texts.emplace_back(from_value<std::string>(info[0]));
    if (info[1].IsString()) {
        texts.emplace_back(from_value<std::string>(info[1]));
        opt = Napi::Config(info[2]);
    } else
        opt = Napi::Config(info[1]);

    bool truncation = opt.Get("truncation", false);
    int32_t max_length = truncation ? opt.Get("max_length", model_max_length) : std::numeric_limits<int32_t>::max();

    for (auto& token : pair_prefix_tokens)
        if (ids.size() < max_length) {
            ids.emplace_back(token);
            types.emplace_back(0);
            masks.emplace_back(1);
        }

    for (i = 0; i < texts.size(); i++) {
        int32_t max_length_ = max_length - pair_suffix_tokens.size();
        special_encode(texts[i], &ids, max_length_);
        if (i < texts.size() - 1)
            for (auto& token : pair_middle_tokens)
                if (ids.size() < max_length_)
                    ids.emplace_back(token);

        for (int32_t j = types.size(); j < ids.size(); j++) {
            types.emplace_back(i);
            masks.emplace_back(1);
        }
    }

    for (auto& token : pair_suffix_tokens)
        if (ids.size() < max_length) {
            ids.emplace_back(token);
            types.emplace_back(i - 1);
            masks.emplace_back(1);
        }

    Napi::Object result = Napi::Object::New(info.Env());
    result.Set("input_ids", to_value(info.Env(), ids));
    result.Set("token_type_ids", to_value(info.Env(), types));
    result.Set("attention_mask", to_value(info.Env(), masks));

    return result;
}

Napi::Value Tokenizer::encode_plus(const Napi::CallbackInfo& info)
{
    if (info[0].IsArray())
        return batch_encode(info);
    else
        return pair_encode(info);
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
