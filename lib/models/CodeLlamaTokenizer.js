const utils = require("./utils");

module.exports = function (tokenizers) {
    class CodeLlamaTokenizer extends tokenizers.LlamaTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "legacy": false,
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    tokenizers.CodeLlamaTokenizer = CodeLlamaTokenizer;
}