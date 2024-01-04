const utils = require("./utils");

module.exports = function (tokenizers) {
    class ElectraTokenizer extends tokenizers.BertTokenizer {
    }

    tokenizers.ElectraTokenizer = ElectraTokenizer;
}