#include "WordpieceTokenizer.h"

Napi::Function JSWordpieceTokenizer::Init(Napi::Env env)
{
    return DefineClass(env, "WordpieceTokenizer",
        { InstanceMethod("tokenize", &JSWordpieceTokenizer::tokenize, napi_enumerable) });
}

JSWordpieceTokenizer::JSWordpieceTokenizer(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<JSWordpieceTokenizer>(info)
{
    vocab_array = to_array<std::u16string>(info[0]);
    for (int i = 0; i < vocab_array.size(); i++)
        vocab_[vocab_array[i]] = i;

    Napi::Config opt(info[1]);
    unk_token_ = opt.Get("unk_token", std::u16string(u"UNK"));
    max_input_chars_per_word_ = opt.Get("max_input_chars_per_word", max_input_chars_per_word_);
}

static std::vector<std::u16string> whitespace_tokenize(std::u16string text)
{
    std::vector<std::u16string> words;
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

Napi::Value JSWordpieceTokenizer::tokenize(const Napi::CallbackInfo& info)
{
    std::vector<std::u16string> words = whitespace_tokenize(from_value<std::u16string>(info[0]));
    std::vector<std::u16string> tokens;

    for (auto itk = words.begin(); itk != words.end(); ++itk) {
        if (static_cast<int64_t>(itk->size()) > max_input_chars_per_word_) {
            tokens.push_back(unk_token_);
            continue;
        }

        bool is_bad = false;
        uint32_t start = 0;
        std::vector<std::u16string> sub_tokens;

        for (; start < itk->size();) {
            uint32_t end = itk->size();
            std::u16string cur_substr;
            uint32_t cur_substr_index = -1;

            for (; start < end;) {
                std::u16string substr = itk->substr(start, end - start);
                if (start > 0)
                    substr = u"##" + substr;

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
