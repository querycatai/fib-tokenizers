const utils = require("./utils");

module.exports = function (tokenizers) {
    class SpeechT5Tokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_eos_token": true,
            };

            super(model, utils.extend(default_config, options));
        }
    }

    SpeechT5Tokenizer.vocabs = [
        "spm_char.model"
    ];

    tokenizers.SpeechT5Tokenizer = SpeechT5Tokenizer;
}