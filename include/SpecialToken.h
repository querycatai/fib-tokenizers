#pragma once

#include <string_view>
#include "napi_value.h"

class SpecialToken {
public:
    SpecialToken()
    {
    }

    SpecialToken(std::string content_, int id_ = -1)
        : content(content_)
        , id(id_)
    {
    }

    SpecialToken(Napi::Value value)
    {
        if (value.IsString()) {
            content = value.As<Napi::String>();
            return;
        }

        Napi::Config opt(value);

        id = opt.Get("id", -1);
        content = opt.Get("content", std::string());
        lstrip = opt.Get("lstrip", true);
        normalized = opt.Get("normalized", false);
        rstrip = opt.Get("rstrip", true);
        single_word = opt.Get("single_word", false);
        special = opt.Get("special", false);
    }

    operator const int&() const
    {
        return id;
    }

    operator const std::string&() const
    {
        return content;
    }

public:
    std::string content;
    bool lstrip = false;
    bool normalized = false;
    bool rstrip = false;
    bool single_word = false;
    bool special = false;
    int id = -1;
};

template <>
inline SpecialToken from_value<SpecialToken>(const Napi::Value& value)
{
    return SpecialToken(value);
}
