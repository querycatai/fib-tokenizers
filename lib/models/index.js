const class_list = [
    "LlamaTokenizer",
    "T5Tokenizer"
];

module.exports = function (tokenizers) {
    class_list.forEach(model => require(`./${model}`)(tokenizers));
}