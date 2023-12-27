module.exports = function (tokenizers) {
    class LlamaTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_bos_token": true
            };
            Object.assign(default_config, options);
            super(model, default_config);
        }
    }

    LlamaTokenizer.vocabs = [
        "tokenizer.model"
    ];

    tokenizers.LlamaTokenizer = LlamaTokenizer;
}