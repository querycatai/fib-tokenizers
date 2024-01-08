const utils = require("./utils");

module.exports = function (tokenizers) {
    class BigBirdTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "bos_token": "<s>",
                "eos_token": "</s>",
                "sep_token": "[SEP]",
                "cls_token": "<cls>",
                "unk_token": "[CLS]",
                "pad_token": "<pad>",
                "mask_token": "[MASK]",
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

    BigBirdTokenizer.vocabs = [
        "spiece.model"
    ];

    tokenizers.BigBirdTokenizer = BigBirdTokenizer;
}