const utils = require("./utils");

module.exports = function (tokenizers) {
    class WhisperTokenizer extends tokenizers.GPT2Tokenizer {
        constructor(vocab, mergs, options) {
            const default_config = {
                "prefix_tokens": [
                    "<|startoftranscript|>",
                    "<|notimestamps|>"
                ],
                "suffix_tokens": [
                    "<|endoftext|>"
                ]
            };

            super(vocab, mergs, utils.extend(default_config, options));
        }
    }

    tokenizers.WhisperTokenizer = WhisperTokenizer;
}