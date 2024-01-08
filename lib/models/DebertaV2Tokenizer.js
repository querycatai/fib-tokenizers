const utils = require("./utils");

module.exports = function (tokenizers) {
    class DebertaV2Tokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "add_bos_token": true,
                "add_eos_token": true,
                "added_tokens_decoder": {
                    "0": "[PAD]",
                    "1": "[CLS]",
                    "2": "[SEP]",
                    "3": "[UNK]",
                },
                "additional_special_tokens": [
                    "[MASK]"
                ],
                "bos_token": "[CLS]",
                "eos_token": "[SEP]",
                "post_processor": {
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

    DebertaV2Tokenizer.vocabs = [
        "spm.model"
    ];

    tokenizers.DebertaV2Tokenizer = DebertaV2Tokenizer;
}