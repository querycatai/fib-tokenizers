const utils = require("./utils");

module.exports = function (tokenizers) {
    class DistilBertTokenizer extends tokenizers.BertTokenizer {
    }

    tokenizers.DistilBertTokenizer = DistilBertTokenizer;
}