#include "TikTokenizer.h"
#include "emdedded_resource_reader.h"

napi_ref JSTikTokenizer::constructor;

napi_value JSTikTokenizer::Init(napi_env env)
{
    napi_property_descriptor properties[] = {
        { "encode", nullptr, encode, nullptr, nullptr, nullptr, napi_enumerable, nullptr },
        { "decode", nullptr, decode, nullptr, nullptr, nullptr, napi_enumerable, nullptr }
    };

    napi_value cons;
    NODE_API_CALL(env, napi_define_class(env, "TikTokenizer", -1, New, nullptr, sizeof(properties) / sizeof(napi_property_descriptor), properties, &cons));
    NODE_API_CALL(env, napi_create_reference(env, cons, 1, &constructor));

    return cons;
}

class LinesReader : public IResourceReader {
public:
    LinesReader(std::vector<std::string>& lines_)
        : lines(lines_)
    {
    }

    std::vector<std::string> readLines() override
    {
        return lines;
    }

private:
    std::vector<std::string>& lines;
};

JSTikTokenizer::JSTikTokenizer(NodeArg<JSTikTokenizer>& arg)
    : env_(arg.env())
{
    NodeOpt opt(arg.args(1));
    LanguageModel base_model = LanguageModel::CL100K_BASE;
    std::string opt_base_model = opt.Get("base_model", std::string("cl100k_base"));
    if (opt_base_model == "cl100k_base")
        base_model = LanguageModel::CL100K_BASE;
    else if (opt_base_model == "r50k_base")
        base_model = LanguageModel::R50K_BASE;
    else if (opt_base_model == "p50k_base")
        base_model = LanguageModel::P50K_BASE;
    else if (opt_base_model == "p50k_edit")
        base_model = LanguageModel::P50K_EDIT;
    else
        NODE_API_ASSERT_RETURN_VOID(env_, false, "Unknown base model");

    std::vector<std::string> lines = arg.args(0);
    LinesReader lines_reader(lines);
    encoder_ = GptEncoding::get_encoding(base_model, &lines_reader);
}

napi_value JSTikTokenizer::encode(napi_env env, napi_callback_info info)
{
    napi_value _this;
    size_t argc = 1;
    napi_value args[1];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    JSTikTokenizer* obj;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

    std::string text = NodeValue(env, args[0]);
    std::vector<int> tokens = obj->encoder_->encode(text);

    return NodeValue(env, tokens);
}

napi_value JSTikTokenizer::decode(napi_env env, napi_callback_info info)
{
    napi_value _this;
    size_t argc = 1;
    napi_value args[1];
    NODE_API_CALL(env, napi_get_cb_info(env, info, &argc, args, &_this, nullptr));

    JSTikTokenizer* obj;
    NODE_API_CALL(env, napi_unwrap(env, _this, reinterpret_cast<void**>(&obj)));

    std::vector<int> ids = NodeValue(env, args[0]);
    std::string text = obj->encoder_->decode(ids);

    return NodeValue(env, text);
}