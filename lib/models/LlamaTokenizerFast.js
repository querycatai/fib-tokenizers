const utils = require("./utils");

module.exports = function (tokenizers) {
    class LlamaTokenizerFast extends tokenizers.FastTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "padding_side": "left"
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    tokenizers.LlamaTokenizerFast = LlamaTokenizerFast;
}