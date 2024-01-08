const utils = require("./utils");

module.exports = function (tokenizers) {
    class XGLMTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
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
                "sep_token": "</s>",
                "pad_token": "<pad>",
                "mask_token": "<mask>",
                "prefix_tokens": [
                    "</s>"
                ],
                "post_processor": {
                    "pair": [
                        {
                            "SpecialToken": {
                                "id": "</s>",
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
                        }
                    ]
                }
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    tokenizers.XGLMTokenizer = XGLMTokenizer;
}