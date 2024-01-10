const utils = require("./utils");

module.exports = function (tokenizers) {
    class QWenTokenizer extends tokenizers.TikTokenizer {
    }

    QWenTokenizer.vocabs = [
        "qwen.tiktoken"
    ];

    tokenizers.QWenTokenizer = QWenTokenizer;
}