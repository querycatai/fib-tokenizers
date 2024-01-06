const utils = require("./utils");

module.exports = function (tokenizers) {
    class OpenAIGPTTokenizer extends tokenizers.BpeTokenizer {
        constructor(vocab, mergs, options) {
            const default_config = {
                "unk_token": "<|endoftext|>",
                "bos_token": "<|startoftext|>",
                "eos_token": "<|endoftext|>",
                "pad_token": "<|endoftext|>",
                "do_lower_case": true,
                "clean_up_spaces": true
            };

            super(vocab, mergs, utils.extend(default_config, options));
        }
    }

    tokenizers.OpenAIGPTTokenizer = OpenAIGPTTokenizer;
}