const utils = require("./utils");

module.exports = function (tokenizers) {
    class CLIPTokenizer extends tokenizers.BpeTokenizer {
        constructor(vocab, mergs, options) {
            const default_config = {
                "add_bos_token": true,
                "add_eos_token": true,
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

    tokenizers.CLIPTokenizer = CLIPTokenizer;
}