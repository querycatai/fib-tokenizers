const utils = require("./utils");

module.exports = function (tokenizers) {
    class GLMGPT2Tokenizer extends tokenizers.BpeTokenizer {
        constructor(vocab, mergs, options) {
            const default_config = {
                "add_eos_token": true,
                "eos_token": "<|endoftext|>",
                "pad_token": "<|endoftext|>",
                "cls_token": "[CLS]",
                "mask_token": "[MASK]",
                "unk_token": "[UNK]",
                "prefix_tokens": [
                    "[CLS]"
                ]
            };

            super(vocab, mergs, utils.extend(default_config, options));
        }
    }

    GLMGPT2Tokenizer.vocabs = [
        "vocab.json", "merges.txt"
    ];

    tokenizers.GLMGPT2Tokenizer = GLMGPT2Tokenizer;
}