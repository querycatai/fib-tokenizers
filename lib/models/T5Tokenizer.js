const utils = require("./utils");

module.exports = function (tokenizers) {
    class T5Tokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_eos_token": true,
                "eos_token": "</s>",
                "unk_token": "<unk>",
                "pad_token": "<pad>"
            };

            super(model, utils.extend(default_config, options));
        }
    }

    T5Tokenizer.vocabs = [
        "spiece.model"
    ];

    tokenizers.T5Tokenizer = T5Tokenizer;
}