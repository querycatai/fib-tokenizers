const utils = require("./utils");

module.exports = function (tokenizers) {
    class LlamaTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_bos_token": true
            };

            super(model, utils.extend(default_config, options));
        }
    }

    LlamaTokenizer.vocabs = [
        "tokenizer.model"
    ];

    tokenizers.LlamaTokenizer = LlamaTokenizer;
}