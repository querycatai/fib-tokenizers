const utils = require("./utils");

module.exports = function (tokenizers) {
    class XLNetTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "bos_token": "<s>",
                "eos_token": "</s>",
                "sep_token": "<sep>",
                "cls_token": "<cls>",
                "unk_token": "<unk>",
                "pad_token": "<pad>",
                "mask_token": "<mask>",
                "additional_special_tokens": ["<eop>", "<eod>"],
                "suffix_tokens": [
                    "<sep>",
                    "<cls>"
                ]
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    XLNetTokenizer.vocabs = [
        "spiece.model"
    ];

    tokenizers.XLNetTokenizer = XLNetTokenizer;
}