#include "WordpieceTokenizer.h"
#include "BasicTokenizer.h"

NAPI_MODULE_INIT()
{
    const napi_property_descriptor exports_set[] = {
        { "BasicTokenizer", nullptr, nullptr, nullptr, nullptr, JSBasicTokenizer::Init(env), napi_enumerable, nullptr },
        { "WordpieceTokenizer", nullptr, nullptr, nullptr, nullptr, JSWordpieceTokenizer::Init(env), napi_enumerable, nullptr }
    };

    NODE_API_CALL(env, napi_define_properties(env, exports, sizeof(exports_set) / sizeof(napi_property_descriptor), exports_set));
    return exports;
}
