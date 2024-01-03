#include "TikTokenizer.h"
#include "string_util.h"
#include "emdedded_resource_reader.h"

Napi::Function JSTikTokenizer::Init(Napi::Env env)
{
    return DefineClass(env, "TikTokenizer",
        { InstanceMethod("tokenize", &JSTikTokenizer::tokenize, napi_default_jsproperty),
            InstanceMethod("encode", &JSTikTokenizer::encode, napi_default_jsproperty),
            InstanceMethod("decode", &JSTikTokenizer::decode, napi_default_jsproperty) });
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
    Napi::Config opt(info[1]);
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

    std::vector<std::string> lines;
    std::string_view vocab_data = from_value<std::string_view>(info[0]);
    split_vocab(vocab_data, lines);

    LinesReader lines_reader(lines);
    encoder_ = GptEncoding::get_encoding(base_model, &lines_reader);
}

Napi::Value JSTikTokenizer::encode(const Napi::CallbackInfo& info)
{
    std::string text = info[0].As<Napi::String>();
    std::vector<int> tokens = encoder_->encode(text);

    return to_value(info.Env(), tokens);
}

Napi::Value JSTikTokenizer::tokenize(const Napi::CallbackInfo& info)
{
    std::string text = info[0].As<Napi::String>();
    std::vector<std::string> tokens = encoder_->tokenize(text);

    return to_value(info.Env(), tokens);
}

Napi::Value JSTikTokenizer::decode(const Napi::CallbackInfo& info)
{
    std::vector<int> ids = to_array<int>(info[0]);
    std::string text = encoder_->decode(ids);

    return to_value(info.Env(), text);
}
