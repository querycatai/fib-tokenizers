const utils = require("./utils");

module.exports = function (tokenizers) {
    class PegasusTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_eos_token": true,
                "offset": 103,
                "added_tokens_decoder": {
                    "0": "<s>",
                    "1": "</s>"
                },
                "eos_token": "</s>"
            };

            super(model, utils.extend(default_config, options));
        }
    }

    PegasusTokenizer.vocabs = [
        "spiece.model"
    ];

    tokenizers.PegasusTokenizer = PegasusTokenizer;
}