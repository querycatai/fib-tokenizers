const utils = require("./utils");

module.exports = function (tokenizers) {
    class YiTokenizer extends tokenizers.LlamaTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "unk_token": "<unk>",
                "additional_special_tokens": [
                    "<|startoftext|>",
                    "<|endoftext|>"
                ]
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    tokenizers.YiTokenizer = YiTokenizer;
}