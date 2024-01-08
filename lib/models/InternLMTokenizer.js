const utils = require("./utils");

module.exports = function (tokenizers) {
    class InternLMTokenizer extends tokenizers.LlamaTokenizer {
        constructor(vocab, options) {
            const default_config = {
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

    tokenizers.InternLMTokenizer = InternLMTokenizer;
}