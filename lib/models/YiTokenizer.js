const utils = require("./utils");

module.exports = function (tokenizers) {
    class YiTokenizer extends tokenizers.LlamaTokenizer {
    }

    tokenizers.YiTokenizer = YiTokenizer;
}