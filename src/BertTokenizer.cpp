#include "BertTokenizer.h"
#include "string_util.h"

Napi::Function JSBertTokenizer::Init(Napi::Env env)
{
    return DefineClass(env, "BertTokenizer",
        { InstanceMethod("tokenize", &JSBertTokenizer::tokenize, napi_enumerable),
            InstanceMethod("encode", &JSBertTokenizer::encode, napi_enumerable),
            InstanceMethod("decode", &JSBertTokenizer::decode, napi_enumerable) });
}

JSBertTokenizer::JSBertTokenizer(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<JSBertTokenizer>(info)
{
    std::string_view vocab_data = from_value<std::string_view>(info[0]);
    split_vocab(vocab_data, vocab_array);

    Napi::Config opt(info[1]);

    do_basic_tokenize_ = opt.Get("do_basic_tokenize", true);
    do_lower_case_ = opt.Get("do_lower_case", true);
    tokenize_chinese_chars_ = opt.Get("tokenize_chinese_chars", true);
    strip_accents_ = opt.Get("strip_accents", false);
    suffix_indicator_ = opt.Get("suffix_indicator", std::u32string(U"##"));
    max_input_chars_per_word_ = opt.Get("max_input_chars_per_word", 100);

    for (int i = 0; i < vocab_array.size(); i++) {
        auto& token = vocab_array[i];
        vocab_.emplace(token, i);

        if (token.rfind(suffix_indicator_, 0) == 0) {
            vocab_array[i] = token.substr(suffix_indicator_.size(), token.size() - suffix_indicator_.size());
            is_substr_.push_back(true);
        } else {
            is_substr_.push_back(false);
        }
    }

    unk_token_ = opt.Get("unk_token", std::u32string(U"[UNK]"));
    FindTokenId(unk_token_, unk_token_id_);

    sep_token_ = opt.Get("sep_token", std::u32string(U"[SEP]"));
    FindTokenId(sep_token_, sep_token_id_);

    pad_token_ = opt.Get("pad_token", std::u32string(U"[PAD]"));
    FindTokenId(pad_token_, pad_token_id_);

    cls_token_ = opt.Get("cls_token", std::u32string(U"[CLS]"));
    FindTokenId(cls_token_, cls_token_id_);

    mask_token_ = opt.Get("mask_token", std::u32string(U"[MASK]"));
    FindTokenId(mask_token_, mask_token_id_);
}

bool JSBertTokenizer::FindTokenId(const std::u32string& token, int32_t& token_id)
{
    auto it = vocab_.find(token);
    if (it == vocab_.end()) {
        return false;
    }

    token_id = it->second;
    return true;
}

void JSBertTokenizer::GreedySearch(const std::u32string& token, std::vector<std::u32string>& tokenized_result)
{
    if (static_cast<int64_t>(token.size()) > max_input_chars_per_word_) {
        tokenized_result.push_back(unk_token_);
        return;
    }

    size_t start = 0;
    size_t end = 0;
    std::u32string substr;
    for (; start < token.size();) {
        end = token.size();
        bool is_found = false;
        // try to found the longest matched sub-token in vocab
        for (; start < end;) {
            substr = static_cast<const std::u32string>(token.substr(start, end - start));
            if (start > 0) {
                substr = static_cast<const std::u32string>(suffix_indicator_ + substr);
            }
            if (FindToken(substr)) {
                is_found = true;
                break;
            }
            end -= 1;
        }
        // token not found in vocab
        if (!is_found) {
            tokenized_result.push_back(unk_token_);
            break;
        }

        tokenized_result.push_back(substr);
        start = end;
    }
}

std::vector<std::u32string> JSBertTokenizer::wordpiece_tokenize(std::u32string& text)
{
    std::vector<std::u32string> result;
    std::u32string token;

    for (auto c : text) {
        if (c == U' ' && !token.empty()) {
            GreedySearch(token, result);
            token.clear();
            continue;
        }

        token.push_back(c);
    }

    if (!token.empty()) {
        GreedySearch(token, result);
    }

    return result;
}

std::vector<std::u32string> JSBertTokenizer::basic_tokenize(std::u32string& text)
{
    std::vector<std::u32string> tokens;
    std::u32string token;

    auto push_current_token_and_clear = [&tokens, &token]() {
        if (!token.empty()) {
            tokens.push_back(token);
            token.clear();
        }
    };

    auto push_single_char_and_clear = [&tokens, &token](char32_t c) {
        token.push_back(c);
        tokens.push_back(token);
        token.clear();
    };

    // strip accent first
    if (strip_accents_) {
        for (auto& c : text) {
            c = StripAccent(c);
        }
    }

    if (do_lower_case_) {
        for (auto& c : text) {
            c = ToLower(c);
        }
    }

    for (auto c : text) {
        if (tokenize_chinese_chars_ && IsCJK(c)) {
            push_current_token_and_clear();
            push_single_char_and_clear(c);
            continue;
        }

        if (strip_accents_ && IsAccent(c)) {
            continue;
        }

        if (tokenize_punctuation_ && IsPunct(c)) {
            push_current_token_and_clear();
            push_single_char_and_clear(c);
            continue;
        }

        if (IsSpace(c)) {
            push_current_token_and_clear();
            continue;
        }

        if (remove_control_chars_ && IsControl(c)) {
            continue;
        }

        token.push_back(c);
    }

    push_current_token_and_clear();

    std::vector<std::u32string> result;
    for (const auto& token : tokens) {
        GreedySearch(token, result);
    }

    return result;
}

Napi::Value JSBertTokenizer::tokenize(const Napi::CallbackInfo& info)
{
    std::u32string text = from_value<std::u32string>(info[0]);
    std::vector<std::u32string> tokens;

    if (do_basic_tokenize_)
        tokens = basic_tokenize(text);
    else
        tokens = wordpiece_tokenize(text);

    return to_value(info.Env(), tokens);
}

Napi::Value JSBertTokenizer::encode(const Napi::CallbackInfo& info)
{
    std::u32string text = from_value<std::u32string>(info[0]);
    std::vector<std::u32string> tokens;

    if (do_basic_tokenize_)
        tokens = basic_tokenize(text);
    else
        tokens = wordpiece_tokenize(text);

    std::vector<int64_t> ids;

    ids.push_back(cls_token_id_);
    for (const auto& token : tokens) {
        int32_t token_id = -1;
        if (!FindTokenId(token, token_id)) {
            ids.push_back(unk_token_id_);
            continue;
        }

        ids.push_back(token_id);
    }
    ids.push_back(sep_token_id_);

    return to_value(info.Env(), ids);
}

bool JSBertTokenizer::RemoveTokenizeSpace(int64_t pre_token_id, int64_t new_token_id)
{
    if (pre_token_id < 0) {
        return true;
    }

    auto pre_char = vocab_array[static_cast<size_t>(pre_token_id)].back();
    auto cur_char = vocab_array[static_cast<size_t>(new_token_id)][0];

    if (cur_char == U'.' || cur_char == U',' || cur_char == U'?' || cur_char == U'!' || cur_char == U'\'') {
        return true;
    }

    // remove both side space
    if (pre_char == U'\'') {
        return true;
    }

    // remove both space beside unicode punctuation
    if (pre_char > 128 && IsPunct(pre_char)) {
        return true;
    }

    if (cur_char > 128 && IsPunct(cur_char)) {
        return true;
    }

    return false;
}

Napi::Value JSBertTokenizer::decode(const Napi::CallbackInfo& info)
{
    std::vector<int64_t> ids = to_array<int64_t>(info[0]);

    bool skip_special_tokens = true;
    bool clean_up_tokenization_spaces = true;

    std::u32string result;
    int64_t pre_token = -1;

    for (auto id : ids) {
        if (skip_special_tokens && (id == unk_token_id_ || id == sep_token_id_ || id == pad_token_id_ || id == cls_token_id_ || id == mask_token_id_)) {
            continue;
        }

        // deal with unk ids
        if (id < 0 || static_cast<size_t>(id) >= vocab_array.size()) {
            if (!result.empty()) {
                result.push_back(' ');
            }
            result.append(unk_token_);
            continue;
        }

        // skip first substr
        if (result.empty() && is_substr_[static_cast<size_t>(id)]) {
            continue;
        }

        if (!(result.empty() || is_substr_[static_cast<size_t>(id)]
                || (clean_up_tokenization_spaces && RemoveTokenizeSpace(pre_token, id)))) {
            result.push_back(' ');
        }

        result.append(vocab_array[static_cast<size_t>(id)]);
        pre_token = id;
    }

    return to_value(info.Env(), result);
}