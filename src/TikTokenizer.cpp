#include "TikTokenizer.h"
#include "string_util.h"
#include "emdedded_resource_reader.h"

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

TikTokenizer::TikTokenizer(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<TikTokenizer>(info)
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

    Tokenizer::init(opt, encoder_->byte_pair_encoding_core_processor_.byte_pair_ranks_.size(), 0);
}

int32_t TikTokenizer::model_token_to_id(std::string_view token)
{
    std::vector<uint8_t> utf8_encoded(token.begin(), token.end());
    auto rank_iter = encoder_->byte_pair_encoding_core_processor_.byte_pair_ranks_.find(utf8_encoded);
    if (rank_iter != encoder_->byte_pair_encoding_core_processor_.byte_pair_ranks_.end())
        return rank_iter->second;

    return 0;
}

void TikTokenizer::encode(std::string_view text, const std::function<void(int32_t, int32_t)>& push_back)
{
    std::string text_str(text);
    std::vector<int32_t> tokens = encoder_->encode(text_str);

    for (int32_t i = 0; i < tokens.size(); i++)
        put_token(tokens[i], i, push_back);
}

void TikTokenizer::encode(std::string_view text, const std::function<void(const std::string&, int32_t)>& push_back)
{
    std::string text_str(text);
    std::vector<std::string> tokens = encoder_->tokenize(text_str);

    for (int32_t i = 0; i < tokens.size(); i++)
        put_token(tokens[i], i, push_back);
}

void TikTokenizer::decode(const std::vector<int32_t>& ids, std::string& text)
{
    text = encoder_->decode(ids);
}
