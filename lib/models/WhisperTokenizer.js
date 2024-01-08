const utils = require("./utils");

module.exports = function (tokenizers) {
    class WhisperTokenizer extends tokenizers.GPT2Tokenizer {
        constructor(vocab, mergs, options) {
            const default_config = {
                "prefix_tokens": [
                    "<|startoftranscript|>",
                    "<|notimestamps|>"
                ],
                "suffix_tokens": [
                    "<|endoftext|>"
                ],
                "post_processor": {
                    "pair": [
                        {
                            "SpecialToken": {
                                "id": "<|startoftranscript|>",
                                "type_id": 0
                            }
                        },
                        {
                            "SpecialToken": {
                                "id": "<|notimestamps|>",
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
                                "id": "<|endoftext|>",
                                "type_id": 0
                            }
                        }
                    ]
                }
            };

            super(vocab, mergs, utils.extend(default_config, options));
        }
    }

    tokenizers.WhisperTokenizer = WhisperTokenizer;
}