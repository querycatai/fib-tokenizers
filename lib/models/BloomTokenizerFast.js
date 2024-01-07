const utils = require("./utils");

module.exports = function (tokenizers) {
    class BloomTokenizerFast extends tokenizers.FastTokenizer {
    }

    tokenizers.BloomTokenizerFast = BloomTokenizerFast;
}