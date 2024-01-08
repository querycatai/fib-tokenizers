const utils = require("./utils");

module.exports = function (tokenizers) {
    class SeamlessM4TTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "offset": 1,
                "added_tokens_decoder": {
                    "0": "<pad>",
                    "1": "<unk>",
                    "2": "<s>",
                    "3": "</s>",
                },
                "legacy": false,
                "unk_token": "<unk>",
                "bos_token": "<s>",
                "eos_token": "</s>",
                "sep_token": "</s>",
                "cls_token": "<s>",
                "pad_token": "<pad>",
                "mask_token": "<mask>",
                "post_processor": {
                    "single": [
                        {
                            "SpecialToken": {
                                "id": "</s>",
                                "type_id": 0
                            }
                        },
                        {
                            "SpecialToken": {
                                "id": "__fra__",
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
                        }
                    ],
                    "pair": [
                        {
                            "SpecialToken": {
                                "id": "__eng__",
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

            super(vocab, utils.extend(default_config, options));
        }
    }

    tokenizers.SeamlessM4TTokenizer = SeamlessM4TTokenizer;
}