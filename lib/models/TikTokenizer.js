const utils = require("./utils");

module.exports = function (tokenizers) {
    class TikTokenizer extends tokenizers.TikTokenizer {
    }

    TikTokenizer.prototype.encode_plus = utils.encode_plus;
    TikTokenizer.vocabs = [
        "qwen.tiktoken"
    ];

    tokenizers.TikTokenizer = TikTokenizer;
}