const utils = require("./utils");

module.exports = function (tokenizers) {
    class CLIPTokenizer extends tokenizers.BpeTokenizer {
        constructor(vocab, mergs, options) {
            const default_config = {
                "add_bos_token": true,
                "add_eos_token": true,
                "unk_token": "<|endoftext|>",
                "bos_token": "<|startoftext|>",
                "eos_token": "<|endoftext|>",
                "pad_token": "<|endoftext|>",
                "do_lower_case": true,
                "clean_up_spaces": true,
                "post_processor": {
                    "pair": [
                        {
                            "SpecialToken": {
                                "id": "<|startoftext|>",
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
                        },
                        {
                            "SpecialToken": {
                                "id": "<|endoftext|>",
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

    tokenizers.CLIPTokenizer = CLIPTokenizer;
}