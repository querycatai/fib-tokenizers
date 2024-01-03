const utils = require("./utils");

module.exports = function (tokenizers) {
    class YiTokenizer extends tokenizers.LlamaTokenizer {
        constructor(model, options) {
            const default_config = {
                "unk_token": "<unk>",
                "additional_special_tokens": [
                    "<|startoftext|>",
                    "<|endoftext|>"
                ]
            };

            super(model, utils.extend(default_config, options));
        }
    }

    tokenizers.YiTokenizer = YiTokenizer;
}