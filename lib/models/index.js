const class_list = [
    "LlamaTokenizer",
    "T5Tokenizer",
    "ChatGLMTokenizer",
    "CodeLlamaTokenizer",
    "DebertaV2Tokenizer",
    "BaichuanTokenizer",
    "XLMRobertaTokenizer"
];

module.exports = function (tokenizers) {
    class_list.forEach(model => require(`./${model}`)(tokenizers));
}