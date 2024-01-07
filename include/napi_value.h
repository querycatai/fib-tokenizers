#pragma once

#include <node_api.h>
#include <napi.h>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include "nlohmann/json.hpp"
#include "utf8.h"

inline Napi::Value to_value(Napi::Env env, const Napi::Value& value)
{
    return value;
}

inline Napi::Value to_value(Napi::Env env, int32_t value)
{
    return Napi::Number::New(env, value);
}

inline Napi::Value to_value(Napi::Env env, uint32_t value)
{
    return Napi::Number::New(env, value);
}

inline Napi::Value to_value(Napi::Env env, int64_t value)
{
    return Napi::Number::New(env, value);
}

inline Napi::Value to_value(Napi::Env env, double value)
{
    return Napi::Number::New(env, value);
}

inline Napi::Value to_value(Napi::Env env, bool value)
{
    return Napi::Boolean::New(env, value);
}

inline Napi::Value to_value(Napi::Env env, const char* value)
{
    return Napi::String::New(env, value);
}

inline Napi::Value to_value(Napi::Env env, const std::string& value)
{
    return Napi::String::New(env, value);
}

inline Napi::Value to_value(Napi::Env env, const std::u16string& value)
{
    return Napi::String::New(env, value);
}

inline Napi::Value to_value(Napi::Env env, const std::u32string& value)
{
    std::string string;

    utf8::convert(value, string);
    return Napi::String::New(env, string);
}

template <typename VALUE_TYPE>
inline Napi::Value to_value(Napi::Env env, const std::vector<VALUE_TYPE>& value)
{
    Napi::Array array = Napi::Array::New(env);

    for (size_t i = 0; i < value.size(); ++i) {
        napi_value element = to_value(env, value[i]);
        array[i] = element;
    }

    return array;
}

template <typename CHAR_TYPE, typename VALUE_TYPE>
inline Napi::Value to_value(Napi::Env env, const std::map<std::basic_string<CHAR_TYPE>, VALUE_TYPE>& value)
{
    Napi::Object object = Napi::Object::New(env);

    for (auto& pair : value)
        object.Set(to_value(pair.first), to_value(pair.second));

    return object;
}

template <typename VALUE_TYPE>
inline VALUE_TYPE from_value(const Napi::Value& value)
{
    return VALUE_TYPE();
}

template <>
inline int32_t from_value<int32_t>(const Napi::Value& value)
{
    return value.As<Napi::Number>().Int32Value();
}

template <>
inline uint32_t from_value<uint32_t>(const Napi::Value& value)
{
    return value.As<Napi::Number>().Uint32Value();
}

template <>
inline int64_t from_value<int64_t>(const Napi::Value& value)
{
    return value.As<Napi::Number>().Int64Value();
}

template <>
inline double from_value<double>(const Napi::Value& value)
{
    return value.As<Napi::Number>().DoubleValue();
}

template <>
inline bool from_value<bool>(const Napi::Value& value)
{
    return value.As<Napi::Boolean>().Value();
}

template <>
inline std::string from_value<std::string>(const Napi::Value& value)
{
    return value.As<Napi::String>();
}

template <>
inline std::u16string from_value<std::u16string>(const Napi::Value& value)
{
    return value.As<Napi::String>();
}

template <>
inline std::u32string from_value<std::u32string>(const Napi::Value& value)
{
    std::u16string u16value(from_value<std::u16string>(value));
    std::u32string result;

    utf8::convert(u16value, result);
    return result;
}

template <>
inline Napi::Value from_value<Napi::Value>(const Napi::Value& value)
{
    return value;
}

template <>
inline std::string_view from_value<std::string_view>(const Napi::Value& value)
{
    Napi::TypedArray typed_array = value.As<Napi::TypedArray>();
    Napi::ArrayBuffer array_buffer = typed_array.ArrayBuffer();
    return std::string_view((const char*)array_buffer.Data(), typed_array.ByteLength());
}

template <typename VALUE_TYPE>
inline std::vector<VALUE_TYPE> to_array(const Napi::Value& value)
{
    std::vector<VALUE_TYPE> result;
    Napi::Array array = value.As<Napi::Array>();
    uint32_t length = array.Length();

    result.resize(length);
    for (uint32_t i = 0; i < length; ++i)
        result[i] = from_value<VALUE_TYPE>(array.Get(i));

    return result;
}

template <typename CHAR_TYPE, typename VALUE_TYPE>
inline std::unordered_map<std::basic_string<CHAR_TYPE>, VALUE_TYPE> to_object(const Napi::Value& value)
{
    std::unordered_map<std::basic_string<CHAR_TYPE>, VALUE_TYPE> result;
    Napi::Object object = value.As<Napi::Object>();
    Napi::Array keys = object.GetPropertyNames();
    uint32_t length = keys.Length();

    for (uint32_t i = 0; i < length; ++i) {
        Napi::Value key = keys.Get(i);
        std::basic_string<CHAR_TYPE> name = from_value<std::basic_string<CHAR_TYPE>>(key);

        result[name] = from_value<VALUE_TYPE>(object.Get(key));
    }

    return result;
}

namespace Napi {

class Config {
public:
    Config(Value opt)
    {
        if (opt.Type() != napi_undefined)
            opt_ = opt.As<Object>();
    }

public:
    template <typename VALUE_TYPE>
    VALUE_TYPE Get(const char* name, VALUE_TYPE default_value) const
    {
        if (!opt_)
            return default_value;

        Value value = opt_.Get(name);

        napi_valuetype valuetype = value.Type();
        if (valuetype == napi_undefined || valuetype == napi_null)
            return default_value;

        VALUE_TYPE result;
        _convert(value, &result);
        return result;
    }

    Value Get(const std::initializer_list<const char*>& names) const
    {
        if (!opt_)
            return Value();

        Value value = opt_;
        for (auto& name : names) {
            Object o_ = value.As<Object>();
            value = o_.Get(name);

            napi_valuetype valuetype = value.Type();
            if (valuetype == napi_undefined || valuetype == napi_null)
                return Value();
        }

        return value;
    }

    void assign(const nlohmann::json& value)
    {
        for (auto& pair : value.items())
            opt_.Set(pair.key(), json_convert(pair.value()));
    }

    Env GetEnv() const
    {
        return opt_.Env();
    }

private:
    Napi::Value json_convert(const nlohmann::json& value)
    {
        switch (value.type()) {
        case nlohmann::json::value_t::null:
            return GetEnv().Null();
        case nlohmann::json::value_t::boolean:
            return Napi::Boolean::New(GetEnv(), value.get<bool>());
        case nlohmann::json::value_t::number_integer:
            return Napi::Number::New(GetEnv(), value.get<int32_t>());
        case nlohmann::json::value_t::number_unsigned:
            return Napi::Number::New(GetEnv(), value.get<uint32_t>());
        case nlohmann::json::value_t::number_float:
            return Napi::Number::New(GetEnv(), value.get<double>());
        case nlohmann::json::value_t::string:
            return Napi::String::New(GetEnv(), value.get<std::string>());
        case nlohmann::json::value_t::array: {
            Napi::Array array = Napi::Array::New(GetEnv(), value.size());

            for (size_t i = 0; i < value.size(); ++i)
                array[i] = json_convert(value[i]);

            return array;
        }
        case nlohmann::json::value_t::object: {
            Napi::Object object = Napi::Object::New(GetEnv());

            for (auto& pair : value.items())
                object.Set(pair.key(), json_convert(pair.value()));

            return object;
        }
        default:
            return GetEnv().Undefined();
        }
    }

    void _convert(const Value& value, Value* result) const
    {
        *result = value;
    }

    template <typename VALUE_TYPE>
    void _convert(const Value& value, VALUE_TYPE* result) const
    {
        *result = from_value<VALUE_TYPE>(value);
    }

    template <typename VALUE_TYPE>
    void _convert(const Value& value, std::vector<VALUE_TYPE>* result) const
    {
        *result = to_array<VALUE_TYPE>(value);
    }

    template <typename CHAR_TYPE, typename VALUE_TYPE>
    void _convert(const Value& value, std::unordered_map<std::basic_string<CHAR_TYPE>, VALUE_TYPE>* result) const
    {
        *result = to_object<CHAR_TYPE, VALUE_TYPE>(value);
    }

private:
    Object opt_;
};

} // namespace Napi
