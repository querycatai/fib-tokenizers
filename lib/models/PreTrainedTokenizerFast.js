const utils = require("./utils");

module.exports = function (tokenizers) {
    class PreTrainedTokenizerFast extends tokenizers.FastTokenizer {
    }

    tokenizers.PreTrainedTokenizerFast = PreTrainedTokenizerFast;
}