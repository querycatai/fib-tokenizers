const utils = require("./utils");

module.exports = function (tokenizers) {
    class SentencepieceTokenizer extends tokenizers.SentencepieceTokenizer {
    }

    // SentencepieceTokenizer.prototype.encode_plus = utils.encode_plus;
    SentencepieceTokenizer.vocabs = [
        "sentencepiece.bpe.model"
    ];

    tokenizers.SentencepieceTokenizer = SentencepieceTokenizer;
}