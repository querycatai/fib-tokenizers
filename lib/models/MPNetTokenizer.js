const utils = require("./utils");

module.exports = function (tokenizers) {
    class MPNetTokenizer extends tokenizers.BertTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "bos_token": "<s>",
                "eos_token": "</s>",
                "unk_token": "[UNK]",
                "sep_token": "</s>",
                "pad_token": "<pad>",
                "cls_token": "<s>",
                "mask_token": "<mask>",
                "additional_special_tokens": [
                    "<unk>"
                ],
                "post_processor": {
                    "single": [
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
                        }
                    ],
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

            super(vocab, utils.extend(default_config, options));
        }
    }

    tokenizers.MPNetTokenizer = MPNetTokenizer;
}