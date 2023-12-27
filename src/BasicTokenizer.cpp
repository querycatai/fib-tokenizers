#include "BasicTokenizer.h"
#include "string_utils.h"

Napi::Function JSBasicTokenizer::Init(Napi::Env env)
{
    return DefineClass(env, "BasicTokenizer",
        { InstanceMethod("tokenize", &JSBasicTokenizer::tokenize, napi_enumerable) });
}

JSBasicTokenizer::JSBasicTokenizer(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<JSBasicTokenizer>(info)
{
    Napi::Config opt(info[0]);
    do_lower_case_ = opt.Get("do_lower_case", true);
    strip_accents_ = opt.Get("strip_accents", true);
    tokenize_chinese_chars_ = opt.Get("tokenize_chinese_chars", true);
    tokenize_punctuation_ = opt.Get("tokenize_punctuation", true);
    remove_control_chars_ = opt.Get("remove_control_chars", true);
}

Napi::Value JSBasicTokenizer::tokenize(const Napi::CallbackInfo& info)
{
    std::string text_utf8 = info[0].As<Napi::String>();
    ustring text(text_utf8);

    std::vector<ustring> result;
    ustring token;
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
