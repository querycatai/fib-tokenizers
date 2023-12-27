#include "TikTokenizer.h"
#include "emdedded_resource_reader.h"

Napi::Function JSTikTokenizer::Init(Napi::Env env)
{
    return DefineClass(env, "TikTokenizer",
        { InstanceMethod("encode", &JSTikTokenizer::encode, napi_enumerable),
            InstanceMethod("decode", &JSTikTokenizer::decode, napi_enumerable) });
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

JSTikTokenizer::JSTikTokenizer(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<JSTikTokenizer>(info)
{
    NodeOpt opt(info[1]);
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
        throw Napi::Error::New(info.Env(), "Unknown base model");

    std::vector<std::string> lines = NodeValue(info[0]);
    LinesReader lines_reader(lines);
    encoder_ = GptEncoding::get_encoding(base_model, &lines_reader);
}

Napi::Value JSTikTokenizer::encode(const Napi::CallbackInfo& info)
{
    std::string text = NodeValue(info[0]);
    std::vector<int> tokens = encoder_->encode(text);

    return NodeValue(info.Env(), tokens);
}

Napi::Value JSTikTokenizer::decode(const Napi::CallbackInfo& info)
{
    std::vector<int> ids = NodeValue(info[0]);
    std::string text = encoder_->decode(ids);

    return NodeValue(info.Env(), text);
}