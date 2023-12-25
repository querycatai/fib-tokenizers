#include "WordpieceTokenizer.h"

napi_ref JSWordpieceTokenizer::constructor;

napi_value JSWordpieceTokenizer::Init(napi_env env)
{
    napi_property_descriptor properties[] = {
        { "tokenize", nullptr, tokenize, nullptr, nullptr, nullptr, napi_enumerable, nullptr }
    };

    napi_value cons;
    NODE_API_CALL(env, napi_define_class(env, "WordpieceTokenizer", -1, New, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NODE_API_CALL(env, napi_create_reference(env, cons, 1, &constructor));

    return cons;
}

JSWordpieceTokenizer::JSWordpieceTokenizer(NodeArg<JSWordpieceTokenizer>& arg)
    : env_(arg.env())
{
    vocab_array = arg.args(0);
    for (int i = 0; i < vocab_array.size(); i++)
        vocab_[vocab_array[i]] = i;

    NodeOpt opt(arg.args(1));
    unk_token_ = opt.Get("unk_token", std::u16string(u"UNK"));
    max_input_chars_per_word_ = opt.Get("max_input_chars_per_word", max_input_chars_per_word_);
}

std::vector<std::u16string> whitespace_tokenize(std::u16string text)
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

napi_value JSWordpieceTokenizer::tokenize(napi_env env, napi_callback_info info)
{
    NodeArg<JSWordpieceTokenizer> obj(env, info);

    std::vector<std::u16string> words = whitespace_tokenize(obj.args(0));
    std::vector<std::u16string> tokens;

    for (auto itk = words.begin(); itk != words.end(); ++itk) {
        if (static_cast<int64_t>(itk->size()) > obj->max_input_chars_per_word_) {
            tokens.push_back(obj->unk_token_);
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

                if (obj->vocab_.find(substr) != obj->vocab_.end()) {
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
            tokens.push_back(obj->unk_token_);
        else
            tokens.insert(tokens.end(), sub_tokens.begin(), sub_tokens.end());
    }

    return NodeValue(env, tokens);
}
