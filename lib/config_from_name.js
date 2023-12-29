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
    if (tokenizer_config)
        Object.assign(config, tokenizer_config);

    if (config.model_type && !config.tokenizer_class)
        config.tokenizer_class = model_map[config.model_type];

    if (config.tokenizer_class)
        config.tokenizer_class = config.tokenizer_class.replace(/Fast$/, "");

    if (!config.tokenizer_class && file_list && file_list.indexOf("vocab.txt") > -1)
        config.tokenizer_class = "BertTokenizer";

    return config;
}
