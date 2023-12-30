#include "BertTokenizer.h"
#include "string_util.h"

Napi::Function JSBertTokenizer::Init(Napi::Env env)
{
    return DefineClass(env, "BertTokenizer",
        { InstanceMethod("tokenize", &JSBertTokenizer::tokenize, napi_enumerable) });
}

static void split_vocab(std::string_view vocab_data, std::vector<std::u32string>& vocab_array)
{
    size_t start = 0;
    size_t pos = 0;

    for (; pos < vocab_data.size(); ++pos) {
        char ch = vocab_data[pos];
        if (ch == '\n') {
            ssize_t end = pos;
            if (end > 0 && vocab_data[end - 1] == '\r')
                end -= 1;

            std::string line(vocab_data.data() + start, end - start);
            std::u32string wline;

            utf8::convert(line, wline);
            vocab_array.push_back(wline);

            start = pos + 1;
        }
    }

    if (pos > start) {
        ssize_t end = pos;
        if (end > 0 && vocab_data[end - 1] == '\r')
            end -= 1;

        std::string line(vocab_data.data() + start, end - start);
        std::u32string wline;

        utf8::convert(line, wline);
        vocab_array.push_back(wline);
    }
}

JSBertTokenizer::JSBertTokenizer(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<JSBertTokenizer>(info)
{
    std::string_view vocab_data = from_value<std::string_view>(info[0]);
    split_vocab(vocab_data, vocab_array);

    for (int i = 0; i < vocab_array.size(); i++)
        vocab_[vocab_array[i]] = i;

    Napi::Config opt(info[1]);
    unk_token_ = opt.Get("unk_token", std::u32string(U"[UNK]"));

    do_basic_tokenize = opt.Get("do_basic_tokenize", false);

    do_lower_case_ = opt.Get("do_lower_case", false);
    strip_accents_ = opt.Get("strip_accents", false);
    tokenize_chinese_chars_ = opt.Get("tokenize_chinese_chars", false);
    tokenize_punctuation_ = opt.Get("tokenize_punctuation", false);
    remove_control_chars_ = opt.Get("remove_control_chars", false);

    max_input_chars_per_word_ = opt.Get("max_input_chars_per_word", max_input_chars_per_word_);
}

static std::vector<std::u32string> whitespace_tokenize(std::u32string text)
{
    std::vector<std::u32string> words;
    size_t pos = 0;
    size_t last = 0;

    for (; pos < text.size(); ++pos) {
        if (text[pos] == ' ') {
            if (last >= 0 && last < pos)
                words.push_back(text.substr(last, pos - last));
            last = pos + 1;
        }
    }

    if (last >= 0 && last < text.size())
        words.push_back(text.substr(last, pos - last));

    return words;
}

Napi::Value JSBertTokenizer::tokenize(const Napi::CallbackInfo& info)
{
    if (do_basic_tokenize)
        return basic_tokenize(info);

    std::u32string text = from_value<std::u32string>(info[0]);

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

    std::vector<std::u32string> words = whitespace_tokenize(text);
    std::vector<std::u32string> tokens;

    for (auto itk = words.begin(); itk != words.end(); ++itk) {
        if (static_cast<int64_t>(itk->size()) > max_input_chars_per_word_) {
            tokens.push_back(unk_token_);
            continue;
        }

        bool is_bad = false;
        uint32_t start = 0;
        std::vector<std::u32string> sub_tokens;

        for (; start < itk->size();) {
            uint32_t end = itk->size();
            std::u32string cur_substr;
            uint32_t cur_substr_index = -1;

            for (; start < end;) {
                std::u32string substr = itk->substr(start, end - start);
                if (start > 0)
                    substr = U"##" + substr;

                if (vocab_.find(substr) != vocab_.end()) {
                    cur_substr = substr;
                    cur_substr_index = start;
                    break;
                }

                end -= 1;
            }

            if (cur_substr_index == -1) {
                is_bad = true;
                break;
            }

            sub_tokens.push_back(cur_substr);
            start = end;
        }

        if (is_bad)
            tokens.push_back(unk_token_);
        else
            tokens.insert(tokens.end(), sub_tokens.begin(), sub_tokens.end());
    }

    return to_value(info.Env(), tokens);
}

Napi::Value JSBertTokenizer::basic_tokenize(const Napi::CallbackInfo& info)
{
    std::u32string text = from_value<std::u32string>(info[0]);

    std::vector<std::u32string> result;
    std::u32string token;
    auto push_current_token_and_clear = [&result, &token]() {
        if (!token.empty()) {
            result.push_back(token);
            token.clear();
        }
    };

    auto push_single_char_and_clear = [&result, &token](char32_t c) {
        token.push_back(c);
        result.push_back(token);
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

        // 0x2019 unicode is not punctuation in some Linux platform,
        // to be consistent, take it as punctuation.
        if (tokenize_punctuation_ && IsPunct(c)) {
            push_current_token_and_clear();
            push_single_char_and_clear(c);
            continue;
        }

        // split by space
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

    return to_value(info.Env(), result);
}
