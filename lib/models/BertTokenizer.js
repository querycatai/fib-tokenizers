const utils = require("./utils");

module.exports = function (tokenizers) {
    class BertTokenizer extends tokenizers.BertTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "unk_token": "[UNK]",
                "sep_token": "[SEP]",
                "pad_token": "[PAD]",
                "cls_token": "[CLS]",
                "mask_token": "[MASK]",
                "model_input_names": [
                    "token_type_ids",
                    "attention_mask"
                ],
                "post_processor": {
                    "single": [
                        {
                            "SpecialToken": {
                                "id": "[CLS]",
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
                                "id": "[SEP]",
                                "type_id": 0
                            }
                        }
                    ],
                    "pair": [
                        {
                            "SpecialToken": {
                                "id": "[CLS]",
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
                                "id": "[SEP]",
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
                                "id": "[SEP]",
                                "type_id": 0
                            }
                        }
                    ]
                }
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    // BertTokenizer.prototype.encode_plus = utils.encode_plus;
    BertTokenizer.vocabs = [
        "vocab.txt"
    ];

    tokenizers.BertTokenizer = BertTokenizer;
}