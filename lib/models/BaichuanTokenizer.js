module.exports = function (tokenizers) {
    class BaichuanTokenizer extends tokenizers.SentencepieceTokenizer {
    }

    BaichuanTokenizer.vocabs = [
        "tokenizer.model"
    ];

    tokenizers.BaichuanTokenizer = BaichuanTokenizer;
}