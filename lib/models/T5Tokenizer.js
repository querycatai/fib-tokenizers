module.exports = function (tokenizers) {
    class T5Tokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_eos_token": true
            };
            Object.assign(default_config, options);
            super(model, default_config);
        }
    }

    T5Tokenizer.vocabs = [
        "spiece.model"
    ];

    tokenizers.T5Tokenizer = T5Tokenizer;
}