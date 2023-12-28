const utils = require("./utils");

module.exports = function (tokenizers) {
    class CodeLlamaTokenizer extends tokenizers.LlamaTokenizer {
        constructor(model, options) {
            const default_config = {
                "legacy": false,
            };

            super(model, utils.extend(default_config, options));
        }
    }

    tokenizers.CodeLlamaTokenizer = CodeLlamaTokenizer;
}