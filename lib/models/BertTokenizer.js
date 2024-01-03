const utils = require("./utils");

module.exports = function (tokenizers) {
    class BertTokenizer extends tokenizers.BertTokenizer {
    }

    BertTokenizer.vocabs = [
        "vocab.txt"
    ];

    tokenizers.BertTokenizer = BertTokenizer;
}