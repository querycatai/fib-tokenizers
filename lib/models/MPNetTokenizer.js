const utils = require("./utils");

module.exports = function (tokenizers) {
    class MPNetTokenizer extends tokenizers.BertTokenizer {
    }

    tokenizers.MPNetTokenizer = MPNetTokenizer;
}