#pragma once

#include <node_api.h>
#include <napi.h>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include "ustring.h"

class NodeValue {
public:
    NodeValue()
    {
    }

    NodeValue(napi_env env, napi_value value)
        : napi_value_(env, value)
    {
    }

    NodeValue(const NodeValue& other)
        : napi_value_(other.napi_value_)
        , type_(other.type_)
    {
    }

    NodeValue(Napi::Value value)
        : napi_value_(value)
    {
    }

    NodeValue(napi_env env, int32_t value)
    {
        napi_value_ = Napi::Number::New(env, value);
    }

    NodeValue(napi_env env, uint32_t value)
    {
        napi_value_ = Napi::Number::New(env, value);
    }

    NodeValue(napi_env env, int64_t value)
    {
        napi_value_ = Napi::Number::New(env, value);
    }

    NodeValue(napi_env env, double value)
    {
        napi_value_ = Napi::Number::New(env, value);
    }

    NodeValue(napi_env env, bool value)
    {
        napi_value_ = Napi::Boolean::New(env, value);
    }

    NodeValue(napi_env env, const char* value)
    {
        napi_value_ = Napi::String::New(env, value);
    }

    NodeValue(napi_env env, const std::string& value)
    {
        napi_value_ = Napi::String::New(env, value);
    }

    NodeValue(napi_env env, const std::u16string& value)
    {
        napi_value_ = Napi::String::New(env, value);
    }

    NodeValue(napi_env env, const ustring& value)
    {
        std::string string(value);
        napi_value_ = Napi::String::New(env, string);
    }

    template <typename VALUE_TYPE>
    NodeValue(napi_env env, const std::vector<VALUE_TYPE>& value)
    {
        Napi::Array array = Napi::Array::New(env);

        for (size_t i = 0; i < value.size(); ++i) {
            napi_value element = NodeValue(env, value[i]);
            array[i] = element;
        }

        napi_value_ = array;
    }

    template <typename CHAR_TYPE, typename VALUE_TYPE>
    NodeValue(napi_env env, const std::unordered_map<std::basic_string<CHAR_TYPE>, VALUE_TYPE>& value)
    {
        Napi::Object object = Napi::Object::New(env);

        for (auto& pair : value)
            object.Set(pair.first, pair.second);

        napi_value_ = object;
    }

public:
    napi_valuetype type()
    {
        if (type_ != (napi_valuetype)-1)
            return type_;

        type_ = napi_value_.Type();
        return type_;
    }

    bool is_boolean()
    {
        return type() == napi_boolean;
    }

    bool is_number()
    {
        return type() == napi_number;
    }

    bool is_string()
    {
        return type() == napi_string;
    }

    bool is_object()
    {
        return type() == napi_object;
    }

    bool is_array()
    {
        return type() == napi_object;
    }

    bool is_function()
    {
        return type() == napi_function;
    }

    bool is_undefined()
    {
        return type() == napi_undefined;
    }

    bool is_null()
    {
        return type() == napi_null;
    }

public:
    operator napi_value() const
    {
        return napi_value_;
    }

    operator Napi::Value() const
    {
        return napi_value_;
    }

    operator int32_t() const
    {
        return napi_value_.As<Napi::Number>().Int32Value();
    }

    operator uint32_t() const
    {
        return napi_value_.As<Napi::Number>().Uint32Value();
    }

    operator int64_t() const
    {
        return napi_value_.As<Napi::Number>().Int64Value();
    }

    operator double() const
    {
        return napi_value_.As<Napi::Number>().DoubleValue();
    }

    operator bool() const
    {
        return napi_value_.As<Napi::Boolean>().Value();
    }

    operator std::string() const
    {
        return napi_value_.As<Napi::String>().Utf8Value();
    }

    operator std::u16string() const
    {
        return napi_value_.As<Napi::String>().Utf16Value();
    }

    operator ustring() const
    {
        ustring result(operator std::string());
        return result;
    }

    operator std::string_view() const
    {
        Napi::TypedArray typed_array = napi_value_.As<Napi::TypedArray>();
        Napi::ArrayBuffer array_buffer = typed_array.ArrayBuffer();
        return std::string_view((const char*)array_buffer.Data(), typed_array.ByteLength());
    }

    template <typename VALUE_TYPE>
    operator std::vector<VALUE_TYPE>() const
    {
        std::vector<VALUE_TYPE> result;
        Napi::Array array = napi_value_.As<Napi::Array>();
        uint32_t length = array.Length();

        result.resize(length);
        for (uint32_t i = 0; i < length; ++i) {
            VALUE_TYPE value = NodeValue(array.Get(i));
            result[i] = value;
        }

        return result;
    }

    template <typename CHAR_TYPE, typename VALUE_TYPE>
    operator std::unordered_map<std::basic_string<CHAR_TYPE>, VALUE_TYPE>() const
    {
        std::unordered_map<std::basic_string<CHAR_TYPE>, VALUE_TYPE> result;
        Napi::Object object = napi_value_.As<Napi::Object>();
        Napi::Array keys = object.GetPropertyNames();
        uint32_t length = keys.Length();

        for (uint32_t i = 0; i < length; ++i) {
            Napi::Value key = keys.Get(i);
            std::basic_string<CHAR_TYPE> name = NodeValue(key);
            VALUE_TYPE index = NodeValue(object.Get(key));
            result[name] = index;
        }

        return result;
    }

public:
    napi_env env() const
    {
        return napi_value_.Env();
    }

private:
    Napi::Value napi_value_;
    napi_valuetype type_ = (napi_valuetype)-1;
};

class NodeOpt {
public:
    NodeOpt(NodeValue opt)
    {
        napi_env env = opt.env();
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, opt, &valuetype);
        if (valuetype != napi_undefined) {
            opt_ = Napi::Value(env, opt).As<Napi::Object>();
            env_ = env;
        }
    }

    NodeOpt(Napi::Value opt)
    {
        napi_env env = opt.Env();
        if (opt.Type() != napi_undefined) {
            opt_ = opt.As<Napi::Object>();
            env_ = env;
        }
    }

public:
    template <typename VALUE_TYPE>
    VALUE_TYPE Get(const char* name, VALUE_TYPE default_value) const
    {
        if (!opt_)
            return default_value;

        napi_value value;
        if (napi_get_named_property(env_, opt_, name, &value) != napi_ok)
            return default_value;

        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env_, value, &valuetype);
        if (valuetype == napi_undefined || valuetype == napi_null)
            return default_value;

        return NodeValue(env_, value);
    }

private:
    napi_env env_ = nullptr;
    Napi::Object opt_;
};
