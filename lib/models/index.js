const class_list = [
    // SentencepieceTokenizer
    "LlamaTokenizer",
    "T5Tokenizer",
    "ChatGLMTokenizer",
    "CodeLlamaTokenizer",
    "DebertaV2Tokenizer",
    "BaichuanTokenizer",
    "XLMRobertaTokenizer",
    "InternLMTokenizer",
    "NllbTokenizer",
    "PegasusTokenizer",
    "MBart50Tokenizer",
    "YiTokenizer",
    "CamembertTokenizer",
    "SpeechT5Tokenizer",
    "SeamlessM4TTokenizer",
    "PlamoTokenizer",
    "MBartTokenizer",
    "SkyworkTokenizer",

    // BertTokenizer
    "DistilBertTokenizer",
    "MPNetTokenizer",
    // TikTokenizer
    "QWenTokenizer"
];

module.exports = function (tokenizers) {
    tokenizers.BertTokenizer.vocabs = [
        "vocab.txt"
    ];

    class_list.forEach(model => require(`./${model}`)(tokenizers));
}