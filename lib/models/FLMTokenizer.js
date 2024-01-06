const utils = require("./utils");

module.exports = function (tokenizers) {
    class FLMTokenizer extends tokenizers.GPT2Tokenizer {
    }

    tokenizers.FLMTokenizer = FLMTokenizer;
}