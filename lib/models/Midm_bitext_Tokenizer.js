const utils = require("./utils");

module.exports = function (tokenizers) {
    class Midm_bitext_Tokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "add_eos_token": true
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    Midm_bitext_Tokenizer.vocabs = [
        "midm_bitext_tokenizer.model"
    ];

    tokenizers.Midm_bitext_Tokenizer = Midm_bitext_Tokenizer;
}