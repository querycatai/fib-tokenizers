const class_list = [
    "LlamaTokenizer",
    "T5Tokenizer",
    "ChatGLMTokenizer",
    "BaichuanTokenizer",
    "XLMRobertaTokenizer"
];

module.exports = function (tokenizers) {
    class_list.forEach(model => require(`./${model}`)(tokenizers));
}