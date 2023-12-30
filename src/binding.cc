#include "WordpieceTokenizer.h"
#include "BertTokenizer.h"
#include "BasicTokenizer.h"
#include "SentencepieceTokenizer.h"
#include "TikTokenizer.h"

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.DefineProperties(
        { Napi::PropertyDescriptor::Value("BasicTokenizer", JSBasicTokenizer::Init(env), napi_enumerable),
            Napi::PropertyDescriptor::Value("BertTokenizer", JSBertTokenizer::Init(env), napi_enumerable),
            Napi::PropertyDescriptor::Value("SentencepieceTokenizer", JSSentencepieceTokenizer::Init(env), napi_enumerable),
            Napi::PropertyDescriptor::Value("WordpieceTokenizer", JSWordpieceTokenizer::Init(env), napi_enumerable),
            Napi::PropertyDescriptor::Value("TikTokenizer", JSTikTokenizer::Init(env), napi_enumerable) });

    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
