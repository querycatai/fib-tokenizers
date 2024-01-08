const utils = require("./utils");

module.exports = function (tokenizers) {
    class XLMRobertaTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "add_bos_token": true,
                "add_eos_token": true,
                "offset": 1,
                "added_tokens_decoder": {
                    "0": "<s>",
                    "1": "<pad>",
                    "2": "</s>",
                    "3": "<unk>",
                },
                "unk_token": "<unk>",
                "bos_token": "<s>",
                "eos_token": "</s>",
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

            super(vocab, utils.extend(default_config, options));
        }
    }

    tokenizers.XLMRobertaTokenizer = XLMRobertaTokenizer;
}