#pragma once

template <typename T>
class NodeClass {
protected:
    static napi_value New(napi_env env, napi_callback_info info)
    {
        napi_value new_target;
        NODE_API_CALL(env, napi_get_new_target(env, info, &new_target));
        NODE_API_ASSERT(env, new_target != nullptr, "Not a constructor call");

        napi_value _this;
        NODE_API_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &_this, nullptr));
        T* obj(new T(env, info));
        napi_wrap(env, _this, obj, Destructor, nullptr, &obj->wrapper_);

        return _this;
    }

    static void Destructor(napi_env env, void* nativeObject, void*)
    {
        T* obj = static_cast<T*>(nativeObject);
        napi_delete_reference(obj->env_, obj->wrapper_);
        delete obj;
    }
};
