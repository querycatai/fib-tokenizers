const utils = require("./utils");

module.exports = function (tokenizers) {
    class SentencepieceTokenizer extends tokenizers.SentencepieceTokenizer {
        encode_plus(text, text_pair, opt) {
            return utils.tensor_wrap(super.encode_plus(text, text_pair, opt), arguments[arguments.length - 1]);
        }
    }

    // SentencepieceTokenizer.prototype.encode_plus = utils.encode_plus;
    SentencepieceTokenizer.vocabs = [
        "sentencepiece.bpe.model"
    ];

    tokenizers.SentencepieceTokenizer = SentencepieceTokenizer;
}