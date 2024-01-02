const utils = require("./utils");

module.exports = function (tokenizers) {
    class PegasusTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_eos_token": true,
                "offset": 103,
                "added_tokens_decoder": {
                    "0": "<pad>",
                    "1": "</s>"
                },
                "eos_token": "</s>"
            };


            utils.extend(default_config, options);

            if (!default_config.additional_special_tokens) {
                default_config.additional_special_tokens = [
                    "<mask_1>",
                    "<mask_2>"
                ];
                for (var i = 2; i < 103; i++)
                    default_config.additional_special_tokens.push(`<unk_${i}>`);
            }

            default_config.additional_special_tokens.forEach((token, index) => {
                default_config.added_tokens_decoder[index + 2] = token;
            });

            super(model, default_config);
        }
    }

    PegasusTokenizer.vocabs = [
        "spiece.model"
    ];

    tokenizers.PegasusTokenizer = PegasusTokenizer;
}