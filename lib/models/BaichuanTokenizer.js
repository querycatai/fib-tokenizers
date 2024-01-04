module.exports = function (tokenizers) {
    class BaichuanTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            console.log(options);
            super(model, options);
        }
    }

    BaichuanTokenizer.vocabs = [
        "tokenizer.model"
    ];

    tokenizers.BaichuanTokenizer = BaichuanTokenizer;
}