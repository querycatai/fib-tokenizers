#include "BasicTokenizer.h"
#include "string_utils.h"

napi_ref JSBasicTokenizer::constructor;

napi_value JSBasicTokenizer::Init(napi_env env)
{
    napi_property_descriptor properties[] = {
        { "tokenize", nullptr, tokenize, nullptr, nullptr, nullptr, napi_enumerable, nullptr }
    };

    napi_value cons;
    NODE_API_CALL(env, napi_define_class(env, "BasicTokenizer", -1, New, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NODE_API_CALL(env, napi_create_reference(env, cons, 1, &constructor));

    return cons;
}

JSBasicTokenizer::JSBasicTokenizer(NodeArg<JSBasicTokenizer>& args)
    : env_(args.env())
{
    NodeOpt opt(args[0]);
    do_lower_case_ = opt.Get("do_lower_case", true);
    strip_accents_ = opt.Get("strip_accents", true);
    tokenize_chinese_chars_ = opt.Get("tokenize_chinese_chars", true);
    tokenize_punctuation_ = opt.Get("tokenize_punctuation", true);
    remove_control_chars_ = opt.Get("remove_control_chars", true);
}

napi_value JSBasicTokenizer::tokenize(napi_env env, napi_callback_info info)
{
    NodeArg<JSBasicTokenizer> args(env, info);

    ustring text = args[0];

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
    if (args->strip_accents_) {
        for (auto& c : text) {
            c = StripAccent(c);
        }
    }

    if (args->do_lower_case_) {
        for (auto& c : text) {
            c = ToLower(c);
        }
    }

    for (auto c : text) {
        if (args->tokenize_chinese_chars_ && IsCJK(c)) {
            push_current_token_and_clear();
            push_single_char_and_clear(c);
            continue;
        }

        if (args->strip_accents_ && IsAccent(c)) {
            continue;
        }

        // 0x2019 unicode is not punctuation in some Linux platform,
        // to be consistent, take it as punctuation.
        if (args->tokenize_punctuation_ && IsPunct(c)) {
            push_current_token_and_clear();
            push_single_char_and_clear(c);
            continue;
        }

        // split by space
        if (IsSpace(c)) {
            push_current_token_and_clear();
            continue;
        }

        if (args->remove_control_chars_ && IsControl(c)) {
            continue;
        }

        token.push_back(c);
    }

    push_current_token_and_clear();

    return NodeValue(env, result);
}
