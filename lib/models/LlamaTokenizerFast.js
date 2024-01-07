const utils = require("./utils");

module.exports = function (tokenizers) {
    class LlamaTokenizerFast extends tokenizers.FastTokenizer {
    }

    tokenizers.LlamaTokenizerFast = LlamaTokenizerFast;
}