#pragma once

#include <node_api.h>
#include "common.h"
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
        : env_(env)
        , value_(value)
    {
    }

    NodeValue(napi_env env, int32_t value)
        : env_(env)
    {
        NODE_API_CALL_RETURN_VOID(env_, napi_create_int32(env_, value, &value_));
    }

    NodeValue(napi_env env, uint32_t value)
        : env_(env)
    {
        NODE_API_CALL_RETURN_VOID(env_, napi_create_uint32(env_, value, &value_));
    }

    NodeValue(napi_env env, int64_t value)
        : env_(env)
    {
        NODE_API_CALL_RETURN_VOID(env_, napi_create_int64(env_, value, &value_));
    }

    NodeValue(napi_env env, double value)
        : env_(env)
    {
        NODE_API_CALL_RETURN_VOID(env_, napi_create_double(env_, value, &value_));
    }

    NodeValue(napi_env env, bool value)
        : env_(env)
    {
        NODE_API_CALL_RETURN_VOID(env_, napi_get_boolean(env_, value, &value_));
    }

    NodeValue(napi_env env, const char* value)
        : env_(env)
    {
        NODE_API_CALL_RETURN_VOID(env_, napi_create_string_utf8(env_, value, NAPI_AUTO_LENGTH, &value_));
    }

    NodeValue(napi_env env, const std::string& value)
        : env_(env)
    {
        NODE_API_CALL_RETURN_VOID(env_, napi_create_string_utf8(env_, value.c_str(), value.size(), &value_));
    }

    NodeValue(napi_env env, const std::u16string& value)
        : env_(env)
    {
        NODE_API_CALL_RETURN_VOID(env_, napi_create_string_utf16(env_, (char16_t*)value.c_str(), value.size(), &value_));
    }

    NodeValue(napi_env env, const ustring& value)
        : env_(env)
    {
        std::string string(value);
        NODE_API_CALL_RETURN_VOID(env_, napi_create_string_utf8(env_, string.c_str(), string.size(), &value_));
    }

    template <typename VALUE_TYPE>
    NodeValue(napi_env env, const std::vector<VALUE_TYPE>& value)
        : env_(env)
    {
        napi_value array;
        NODE_API_CALL_RETURN_VOID(env_, napi_create_array(env_, &array));

        for (size_t i = 0; i < value.size(); ++i) {
            napi_value element = NodeValue(env_, value[i]);
            NODE_API_CALL_RETURN_VOID(env_, napi_set_element(env_, array, i, element));
        }

        value_ = array;
    }

    template <typename CHAR_TYPE, typename VALUE_TYPE>
    NodeValue(napi_env env, const std::unordered_map<std::basic_string<CHAR_TYPE>, VALUE_TYPE>& value)
        : env_(env)
    {
        napi_value object;
        NODE_API_CALL_RETURN_VOID(env_, napi_create_object(env_, &object));

        for (auto& pair : value) {
            napi_value key = NodeValue(env_, pair.first);
            napi_value element = NodeValue(env_, pair.second);
            NODE_API_CALL_RETURN_VOID(env_, napi_set_property(env_, object, key, element));
        }

        value_ = object;
    }

public:
    operator napi_value() const
    {
        return value_;
    }

    operator int32_t() const
    {
        int32_t result;
        NODE_API_CALL(env_, napi_get_value_int32(env_, value_, &result));
        return result;
    }

    operator uint32_t() const
    {
        uint32_t result;
        NODE_API_CALL(env_, napi_get_value_uint32(env_, value_, &result));
        return result;
    }

    operator int64_t() const
    {
        int64_t result;
        NODE_API_CALL(env_, napi_get_value_int64(env_, value_, &result));
        return result;
    }

    operator double() const
    {
        double result;
        NODE_API_CALL(env_, napi_get_value_double(env_, value_, &result));
        return result;
    }

    operator bool() const
    {
        bool result;
        NODE_API_CALL(env_, napi_get_value_bool(env_, value_, &result));
        return result;
    }

    operator std::string() const
    {
        std::string result;
        size_t size;
        NODE_API_CALL_BASE(env_, napi_get_value_string_utf8(env_, value_, nullptr, 0, &size), result);
        result.resize(size);
        char* data = result.data();
        NODE_API_CALL_BASE(env_, napi_get_value_string_utf8(env_, value_, data, size + 1, nullptr), result);
        return result;
    }

    operator std::u16string() const
    {
        std::u16string result;
        size_t size;
        NODE_API_CALL_BASE(env_, napi_get_value_string_utf16(env_, value_, nullptr, 0, &size), result);
        result.resize(size);
        char16_t* data = result.data();
        NODE_API_CALL_BASE(env_, napi_get_value_string_utf16(env_, value_, data, size + 1, nullptr), result);
        return result;
    }

    operator ustring() const
    {
        ustring result(operator std::string());
        return result;
    }

    operator std::string_view() const
    {
        napi_typedarray_type type;
        napi_value input_buffer;
        size_t byte_offset;
        size_t i, length;
        NODE_API_CALL_BASE(env_, napi_get_typedarray_info(env_, value_, &type, &length, NULL, &input_buffer, &byte_offset), std::string_view());
        NODE_API_ASSERT_BASE(env_, type == napi_uint8_array, "Wrong argument type, must be Uint8Array", std::string_view());

        void* data;
        size_t byte_length;
        NODE_API_CALL_BASE(env_, napi_get_arraybuffer_info(env_, input_buffer, &data, &byte_length), std::string_view());

        return std::string_view((char*)data + byte_offset, length);
    }

    template <typename VALUE_TYPE>
    operator std::vector<VALUE_TYPE>() const
    {
        std::vector<VALUE_TYPE> result;
        napi_value array;
        NODE_API_CALL_BASE(env_, napi_coerce_to_object(env_, value_, &array), result);
        uint32_t length;
        NODE_API_CALL_BASE(env_, napi_get_array_length(env_, array, &length), result);

        result.resize(length);
        for (uint32_t i = 0; i < length; ++i) {
            napi_value element;
            NODE_API_CALL_BASE(env_, napi_get_element(env_, array, i, &element), result);
            VALUE_TYPE value = NodeValue(env_, element);
            result[i] = value;
        }

        return result;
    }

    template <typename CHAR_TYPE, typename VALUE_TYPE>
    operator std::unordered_map<std::basic_string<CHAR_TYPE>, VALUE_TYPE>() const
    {
        std::unordered_map<std::basic_string<CHAR_TYPE>, VALUE_TYPE> result;
        napi_value object;
        NODE_API_CALL_BASE(env_, napi_coerce_to_object(env_, value_, &object), result);
        napi_value keys;
        NODE_API_CALL_BASE(env_, napi_get_property_names(env_, object, &keys), result);
        uint32_t length;
        NODE_API_CALL_BASE(env_, napi_get_array_length(env_, keys, &length), result);

        for (uint32_t i = 0; i < length; ++i) {
            napi_value key;
            NODE_API_CALL_BASE(env_, napi_get_element(env_, keys, i, &key), result);
            std::basic_string<CHAR_TYPE> name = NodeValue(env_, key);
            napi_value value;
            NODE_API_CALL_BASE(env_, napi_get_property(env_, object, key, &value), result);
            VALUE_TYPE index = NodeValue(env_, value);
            result[name] = index;
        }

        return result;
    }

public:
    napi_env env() const
    {
        return env_;
    }

private:
    napi_env env_;
    napi_value value_;
};

class NodeOpt {
public:
    NodeOpt(NodeValue opt)
        : env_(opt.env())
    {
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env_, opt, &valuetype);
        if (valuetype != napi_undefined) {
            NODE_API_CALL_RETURN_VOID(env_, napi_coerce_to_object(env_, opt, &opt_));
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
    napi_env env_;
    napi_value opt_ = nullptr;
};

template <typename T>
class NodeArg {
public:
    NodeArg(napi_env env, napi_callback_info info)
        : env_(env)
    {

        size_t argc = 0;
        NODE_API_CALL_RETURN_VOID(env_, napi_get_cb_info(env_, info, &argc, nullptr, &this_, nullptr));

        if (argc) {
            args_.resize(argc);
            NODE_API_CALL_RETURN_VOID(env_, napi_get_cb_info(env_, info, &argc, args_.data(), nullptr, nullptr));
        }
    }

    T* operator->()
    {
        if (!obj) {
            NODE_API_CALL(env_, napi_unwrap(env_, this_, reinterpret_cast<void**>(&obj)));
        }

        return obj;
    }

    napi_env env() const
    {
        return env_;
    }

    napi_value This() const
    {
        return this_;
    }

    size_t argc() const
    {
        return args_.size();
    }

    NodeValue operator[](size_t index) const
    {
        if (index >= args_.size())
            return NodeValue(env_, napi_value(nullptr));

        return NodeValue(env_, args_[index]);
    }

private:
    napi_env env_;
    napi_value this_ = nullptr;
    T* obj = nullptr;
    std::vector<napi_value> args_;
};

template <typename T>
class NodeClass {
protected:
    static napi_value New(napi_env env, napi_callback_info info)
    {
        napi_value new_target;
        NODE_API_CALL(env, napi_get_new_target(env, info, &new_target));
        NODE_API_ASSERT(env, new_target != nullptr, "Not a constructor call");

        NodeArg<T> args(env, info);
        T* obj(new T(args));
        napi_wrap(env, args.This(), obj, Destructor, nullptr, &obj->wrapper_);

        return args.This();
    }

    static void Destructor(napi_env env, void* nativeObject, void*)
    {
        T* obj = static_cast<T*>(nativeObject);
        napi_delete_reference(obj->env_, obj->wrapper_);
        delete obj;
    }
};
