const utils = require("./utils");

module.exports = function (tokenizers) {
    class BpeTokenizer extends tokenizers.BpeTokenizer {
    }

    BpeTokenizer.prototype.encode_plus = utils.encode_plus;
    BpeTokenizer.vocabs = [
        "vocab.json", "merges.txt"
    ];

    tokenizers.BpeTokenizer = BpeTokenizer;
}