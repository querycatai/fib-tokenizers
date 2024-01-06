const utils = require("./utils");

module.exports = function (tokenizers) {
    class GPT2Tokenizer extends tokenizers.BpeTokenizer {
        constructor(vocab, mergs, options) {
            const default_config = {
                "unk_token": "<|endoftext|>",
                "bos_token": "<|endoftext|>",
                "eos_token": "<|endoftext|>"
            };

            super(vocab, mergs, utils.extend(default_config, options));
        }
    }

    tokenizers.GPT2Tokenizer = GPT2Tokenizer;
}