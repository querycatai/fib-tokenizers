const utils = require("./utils");

module.exports = function (tokenizers) {
    class BpeTokenizer extends tokenizers.BpeTokenizer {
        encode_plus(text, text_pair, opt) {
            return utils.tensor_wrap(super.encode_plus(text, text_pair, opt), arguments[arguments.length - 1]);
        }
    }

    // BpeTokenizer.prototype.encode_plus = utils.encode_plus;
    BpeTokenizer.vocabs = [
        "vocab.json", "merges.txt"
    ];

    tokenizers.BpeTokenizer = BpeTokenizer;
}