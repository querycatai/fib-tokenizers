const utils = require("./utils");

module.exports = function (tokenizers) {
    class PegasusTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "add_eos_token": true,
                "offset": 103,
                "added_tokens_decoder": {
                    "0": "<pad>",
                    "1": "</s>",
                    "2": "<mask_1>",
                    "3": "<mask_2>"
                },
                "eos_token": "</s>",
                "post_processor": {
                    "pair": [
                        {
                            "Sequence": {
                                "id": "A",
                                "type_id": 0
                            }
                        },
                        {
                            "Sequence": {
                                "id": "B",
                                "type_id": 0
                            }
                        },
                        {
                            "SpecialToken": {
                                "id": "</s>",
                                "type_id": 0
                            }
                        }
                    ]
                }
            };


            utils.extend(default_config, options);

            if (!default_config.additional_special_tokens) {
                default_config.additional_special_tokens = [];
                for (var i = 2; i < 103; i++)
                    default_config.additional_special_tokens.push(`<unk_${i}>`);
            } else {
                default_config.additional_special_tokens = default_config.additional_special_tokens.filter((token) => {
                    return token != "<mask_1>" && token != "<mask_2>";
                });
            }

            default_config.additional_special_tokens.forEach((token, index) => {
                default_config.added_tokens_decoder[index + 4] = token;
            });

            super(vocab, default_config);
        }
    }

    PegasusTokenizer.vocabs = [
        "spiece.model"
    ];

    tokenizers.PegasusTokenizer = PegasusTokenizer;
}