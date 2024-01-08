const utils = require("./utils");

module.exports = function (tokenizers) {
    class SpeechT5Tokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "add_eos_token": true,
                "post_processor": {
                    "pair": [
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

    SpeechT5Tokenizer.vocabs = [
        "spm_char.model"
    ];

    tokenizers.SpeechT5Tokenizer = SpeechT5Tokenizer;
}