#include "WordpieceTokenizer.h"
#include "BasicTokenizer.h"
#include "SentencepieceTokenizer.h"
#include "TikTokenizer.h"

Napi::Object Init(Napi::Env env, Napi::Object exports)
{
    exports.DefineProperties(
        { Napi::PropertyDescriptor::Value("BasicTokenizer", JSBasicTokenizer::Init(env)),
            Napi::PropertyDescriptor::Value("WordpieceTokenizer", JSWordpieceTokenizer::Init(env)),
            Napi::PropertyDescriptor::Value("SentencepieceTokenizer", JSSentencepieceTokenizer::Init(env)),
            Napi::PropertyDescriptor::Value("TikTokenizer", JSTikTokenizer::Init(env)) });

    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
