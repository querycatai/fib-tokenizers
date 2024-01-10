const utils = require("./utils");

module.exports = function (tokenizers) {
    class XLNetTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "bos_token": "<s>",
                "eos_token": "</s>",
                "sep_token": "<sep>",
                "cls_token": "<cls>",
                "unk_token": "<unk>",
                "pad_token": "<pad>",
                "mask_token": "<mask>",
                "padding_side": "left",
                "additional_special_tokens": ["<eop>", "<eod>"],
                "model_input_names": [
                    "token_type_ids",
                    "attention_mask"
                ],
                "post_processor": {
                    "single": [
                        {
                            "Sequence": {
                                "id": "A",
                                "type_id": 0
                            }
                        },
                        {
                            "SpecialToken": {
                                "id": "<sep>",
                                "type_id": 0
                            }
                        },
                        {
                            "SpecialToken": {
                                "id": "<cls>",
                                "type_id": 0
                            }
                        }
                    ],
                    "pair": [
                        {
                            "Sequence": {
                                "id": "A",
                                "type_id": 0
                            }
                        },
                        {
                            "SpecialToken": {
                                "id": "<sep>",
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
                                "id": "<sep>",
                                "type_id": 0
                            }
                        },
                        {
                            "SpecialToken": {
                                "id": "<cls>",
                                "type_id": 0
                            }
                        }
                    ]
                }
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    XLNetTokenizer.vocabs = [
        "spiece.model"
    ];

    tokenizers.XLNetTokenizer = XLNetTokenizer;
}