const utils = require("./utils");

module.exports = function (tokenizers) {
    class InternLMTokenizer extends tokenizers.LlamaTokenizer {
    }

    tokenizers.InternLMTokenizer = InternLMTokenizer;
}