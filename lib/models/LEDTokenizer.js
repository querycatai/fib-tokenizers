const utils = require("./utils");

module.exports = function (tokenizers) {
    class LEDTokenizer extends tokenizers.BartTokenizer {
    }

    tokenizers.LEDTokenizer = LEDTokenizer;
}