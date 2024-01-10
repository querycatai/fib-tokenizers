const utils = require("./utils");

module.exports = function (tokenizers) {
    class ChatGLMTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "padding_side": "left",
                "additional_special_tokens": [
                    "[MASK]",
                    "[gMASK]",
                    "[sMASK]",
                    "sop",
                    "eop"
                ],
                "model_input_names": [
                    "attention_mask",
                    "position_ids"
                ],
                "post_processor": {
                    "single": [
                        {
                            "SpecialToken": {
                                "id": "[gMASK]",
                                "type_id": 0
                            }
                        },
                        {
                            "SpecialToken": {
                                "id": "sop",
                                "type_id": 0
                            }
                        },
                        {
                            "Sequence": {
                                "id": "A",
                                "type_id": 0
                            }
                        }
                    ],
                    "pair": [
                        {
                            "SpecialToken": {
                                "id": "[gMASK]",
                                "type_id": 0
                            }
                        },
                        {
                            "SpecialToken": {
                                "id": "sop",
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

    ChatGLMTokenizer.vocabs = [
        "tokenizer.model"
    ];

    tokenizers.ChatGLMTokenizer = ChatGLMTokenizer;
}