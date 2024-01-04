#include "BertTokenizer.h"
#include "string_util.h"

BertTokenizer::BertTokenizer(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<BertTokenizer>(info)
    , Tokenizer(false)
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
        vocab_.insert_or_assign(token, i);

        if (token.rfind(suffix_indicator_, 0) == 0) {
            vocab_array[i] = token.substr(suffix_indicator_.size(), token.size() - suffix_indicator_.size());
            is_substr_.push_back(true);
        } else {
            is_substr_.push_back(false);
        }
    }

    unk_token_ = opt.Get("unk_token", std::u32string(U"[UNK]"));
    FindTokenId(unk_token_, unk_token_id_);

    Tokenizer::init(opt, vocab_array.size(), unk_token_id_);
}

bool BertTokenizer::FindTokenId(const std::u32string& token, int32_t& token_id)
{
    auto it = vocab_.find(token);
    if (it == vocab_.end()) {
        return false;
    }

    token_id = it->second;
    return true;
}

void BertTokenizer::GreedySearch(const std::u32string& token, std::vector<std::u32string>& tokenized_result)
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

std::vector<std::u32string> BertTokenizer::wordpiece_tokenize(std::u32string& text)
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

std::vector<std::u32string> BertTokenizer::basic_tokenize(std::u32string& text)
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

bool BertTokenizer::RemoveTokenizeSpace(int64_t pre_token_id, int64_t new_token_id)
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

int32_t BertTokenizer::model_token_to_id(std::string_view token)
{
    std::u32string token32;
    utf8::convert(token.data(), token.size(), token32);

    auto it = vocab_.find(token32);
    if (it == vocab_.end()) {
        return unk_token_id_;
    }

    return it->second;
}

void BertTokenizer::encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back)
{
    std::u32string text32;
    utf8::convert(text.data(), text.size(), text32);

    std::vector<std::u32string> tokens;

    if (do_basic_tokenize_)
        tokens = basic_tokenize(text32);
    else
        tokens = wordpiece_tokenize(text32);

    for (int32_t i = 0; i < tokens.size(); i++) {
        int32_t token_id = -1;
        if (!FindTokenId(tokens[i], token_id)) {
            push_back(unk_token_id_, i);
            continue;
        }

        push_back(token_id, i);
    }
}

void BertTokenizer::encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back)
{
    std::u32string text32;
    utf8::convert(text.data(), text.size(), text32);

    std::vector<std::u32string> tokens;

    if (do_basic_tokenize_)
        tokens = basic_tokenize(text32);
    else
        tokens = wordpiece_tokenize(text32);

    for (int32_t i = 0; i < tokens.size(); i++) {
        std::string token_str;
        utf8::convert(tokens[i], token_str);
        push_back(token_str, i);
    }
}

void BertTokenizer::decode(const std::vector<int32_t>& ids, std::string& text)
{
    bool clean_up_tokenization_spaces = true;

    std::u32string result;
    int64_t pre_token = -1;

    for (auto id : ids) {
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

    utf8::convert(result, text);
}
