const class_list = [
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
    "SpeechT5Tokenizer"
];

module.exports = function (tokenizers) {
    tokenizers.BertTokenizer.vocabs = [
        "vocab.txt"
    ];

    class_list.forEach(model => require(`./${model}`)(tokenizers));
}