const utils = require("./utils");

module.exports = function (tokenizers) {
    class GPT2TokenizerFast extends tokenizers.FastTokenizer {
    }

    tokenizers.GPT2TokenizerFast = GPT2TokenizerFast;
}