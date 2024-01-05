const utils = require("./utils");

module.exports = function (tokenizers) {
    class CodeGenTokenizer extends tokenizers.GPT2Tokenizer {
    }

    tokenizers.CodeGenTokenizer = CodeGenTokenizer;
}