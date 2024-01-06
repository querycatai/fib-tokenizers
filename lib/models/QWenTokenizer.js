const utils = require("./utils");

module.exports = function (tokenizers) {
    class QWenTokenizer extends tokenizers.TikTokenizer {
    }

    tokenizers.QWenTokenizer = QWenTokenizer;
}