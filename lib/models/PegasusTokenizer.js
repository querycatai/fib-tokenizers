const utils = require("./utils");

module.exports = function (tokenizers) {
    class PegasusTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_eos_token": true,
                "offset": 103,
                "special_token_offset": 2,
                "added_tokens_decoder": {
                    "0": "<pad>",
                    "1": "</s>"
                },
                "additional_special_tokens": [
                    "<mask_2>",
                    "<mask_1>"
                ],
                "eos_token": "</s>"
            };

            for (var i = 2; i < 103; i++)
                default_config.additional_special_tokens.push(`<unk_${i}>`);

            super(model, utils.extend(default_config, options));
        }
    }

    PegasusTokenizer.vocabs = [
        "spiece.model"
    ];

    tokenizers.PegasusTokenizer = PegasusTokenizer;
}