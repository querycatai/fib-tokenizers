const utils = require("./utils");

module.exports = function (tokenizers) {
    class LlamaTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "add_bos_token": true
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    LlamaTokenizer.vocabs = [
        "tokenizer.model"
    ];

    tokenizers.LlamaTokenizer = LlamaTokenizer;
}