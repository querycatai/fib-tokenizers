const utils = require("./utils");

module.exports = function (tokenizers) {
    class RobertaTokenizer extends tokenizers.BpeTokenizer {
        constructor(vocab, mergs, options) {
            const default_config = {
                "add_bos_token": true,
                "add_eos_token": true,
                "bos_token": "<s>",
                "eos_token": "</s>",
                "sep_token": "</s>",
                "cls_token": "<s>",
                "unk_token": "<unk>",
                "pad_token": "<pad>",
                "mask_token": "<mask>",
                "post_processor": {
                    "pair": [
                        {
                            "SpecialToken": {
                                "id": "<s>",
                                "type_id": 0
                            }
                        },
                        {
                            "Sequence": {
                                "id": "A",
                                "type_id": 0
                            }
                        },
                        {
                            "SpecialToken": {
                                "id": "</s>",
                                "type_id": 0
                            }
                        },
                        {
                            "SpecialToken": {
                                "id": "</s>",
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

            super(vocab, mergs, utils.extend(default_config, options));
        }
    }

    tokenizers.RobertaTokenizer = RobertaTokenizer;
}