#include "FastTokenizer.h"
#include "BertTokenizer.h"
#include "BpeTokenizer.h"

FastTokenizer::FastTokenizer(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<FastTokenizer>(info)
{
    Napi::Config opt(info[1]);

    nlohmann::json vocab = nlohmann::json::parse(from_value<std::string_view>(info[0]));
    nlohmann::json model = vocab["model"];
    vocab.erase("model");
    opt.assign(vocab);

    const nlohmann::json model_type = model["type"];
    std::string model_type_str = "BPE";
    if (model_type.is_string())
        model_type_str = model_type.get<std::string>();

    if (model_type_str == "BPE") {
        std::map<std::string, int32_t> vocab_map_ = model["vocab"].get<std::map<std::string, int32_t>>();
        model.erase("vocab");

        std::vector<std::string> merges = model["merges"].get<std::vector<std::string>>();
        model.erase("merges");

        opt.assign(model);

        Tokenizer::init(std::make_shared<BpeTokenizerCore>(vocab_map_, merges, opt), opt);
    } else if (model_type_str == "WordPiece") {
        std::map<std::string, int32_t> vocab_map_ = model["vocab"].get<std::map<std::string, int32_t>>();
        model.erase("vocab");

        opt.assign(model);
        opt.Set("do_basic_tokenize", false);

        Tokenizer::init(std::make_shared<BertTokenizerCore>(vocab_map_, opt), opt);
    } else if (model_type_str == "Bert") {
    } else {
        throw Napi::Error::New(opt.GetEnv(), model_type_str + " is not supported");
    }
}
