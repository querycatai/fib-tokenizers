const fs = require("fs");
const process = require("process");
const path = require("path");

const model_map = {
    "bart": "BartTokenizer",
    "bert": "BertTokenizer",
    "biogpt": "BioGptTokenizer",
    "blenderbot": "BlenderbotTokenizer",
    "clip": "CLIPTokenizer",
    "deberta-v2": "DebertaV2Tokenizer",
    "distilbert": "DistilBertTokenizer",
    "gpt_neo": "GPT2Tokenizer",
    "gpt2": "GPT2Tokenizer",
    "longformer": "LongformerTokenizer",
    "m2m_100": "M2M100Tokenizer",
    "marian": "MarianTokenizer",
    "openai-gpt": "OpenAIGPTTokenizer",
    "opt": "GPT2Tokenizer",
    "pegasus": "PegasusTokenizer",
    "roberta": "RobertaTokenizer",
    "t5": "T5Tokenizer",
    "wav2vec2": "Wav2Vec2CTCTokenizer",
    "xlm-roberta": "XLMRobertaTokenizer"
};

module.exports = function (config, tokenizer_config, file_list) {
    if (typeof config === 'string') {
        const model_home = path.join(process.env.HOME, ".cache/huggingface/hub", "models--" + config.replace(/\//g, "--"));
        const tag = fs.readFileSync(path.join(model_home, "refs/main"), "utf-8");

        const model_path = path.join(model_home, "snapshots", tag);

        if (fs.existsSync(path.join(model_path, "config.json")))
            config = JSON.parse(fs.readFileSync(path.join(model_path, "config.json"), "utf-8"));
        if (fs.existsSync(path.join(model_path, "tokenizer_config.json")))
            tokenizer_config = JSON.parse(fs.readFileSync(path.join(model_path, "tokenizer_config.json"), "utf-8"));
        file_list = fs.readdirSync(model_path);
    }

    var model_type;
    var tokenizer_class;

    if (config) {
        model_type = config.model_type;
        tokenizer_class = config.tokenizer_class;
    }

    if (!tokenizer_class && tokenizer_config)
        tokenizer_class = tokenizer_config.tokenizer_class;

    if (model_type && !tokenizer_class)
        tokenizer_class = model_map[model_type];

    if (tokenizer_class)
        tokenizer_class = tokenizer_class.replace(/Fast$/, "");

    if (!tokenizer_class && file_list && file_list.indexOf("vocab.txt") > -1)
        tokenizer_class = "BertTokenizer";

    return tokenizer_class;
}
