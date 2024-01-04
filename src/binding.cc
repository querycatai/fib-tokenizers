#include "BertTokenizer.h"
#include "SentencepieceTokenizer.h"
#include "TikTokenizer.h"

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.DefineProperties(
        { Napi::PropertyDescriptor::Value("BertTokenizer", JSBertTokenizer::Init(env), napi_default_jsproperty),
            Napi::PropertyDescriptor::Value("SentencepieceTokenizer", SentencepieceTokenizer::Init(env), napi_default_jsproperty),
            Napi::PropertyDescriptor::Value("TikTokenizer", JSTikTokenizer::Init(env), napi_default_jsproperty) });

    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
