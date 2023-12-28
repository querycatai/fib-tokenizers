module.exports = function (tokenizers) {
    class DebertaV2Tokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_bos_token": true,
                "add_eos_token": true,
            };
            Object.assign(default_config, options);
            super(model, default_config);
        }
    }

    DebertaV2Tokenizer.vocabs = [
        "spm.model"
    ];

    tokenizers.DebertaV2Tokenizer = DebertaV2Tokenizer;
}