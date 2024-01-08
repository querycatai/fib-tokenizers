const utils = require("./utils");

module.exports = function (tokenizers) {
    class WhisperTokenizer extends tokenizers.GPT2Tokenizer {
        constructor(vocab, mergs, options) {
            const default_config = {
                "post_processor": {
                    "single": [
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
                            "SpecialToken": {
                                "id": "<|endoftext|>",
                                "type_id": 0
                            }
                        }
                    ],
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