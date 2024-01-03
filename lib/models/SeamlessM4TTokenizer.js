const utils = require("./utils");

module.exports = function (tokenizers) {
    class SeamlessM4TTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_eos_token": true,
                "offset": 1,
                "added_tokens_decoder": {
                    "0": "<pad>",
                    "1": "<unk>",
                    "2": "<s>",
                    "3": "</s>",
                },
                "legacy": false,
                "unk_token": "<unk>",
                "bos_token": "<s>",
                "eos_token": "</s>",
                "sep_token": "</s>",
                "cls_token": "<s>",
                "pad_token": "<pad>",
                "mask_token": "<mask>",
                "prefix_tokens": [
                    "</s>",
                    "__fra__"
                ]
            };

            super(model, utils.extend(default_config, options));
        }
    }

    SeamlessM4TTokenizer.vocabs = [
        "sentencepiece.bpe.model"
    ];

    tokenizers.SeamlessM4TTokenizer = SeamlessM4TTokenizer;
}