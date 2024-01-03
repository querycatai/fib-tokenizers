const utils = require("./utils");

module.exports = function (tokenizers) {
    class BigBirdTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "bos_token": "<s>",
                "eos_token": "</s>",
                "sep_token": "[SEP]",
                "cls_token": "<cls>",
                "unk_token": "[CLS]",
                "pad_token": "<pad>",
                "mask_token": "[MASK]",
                "prefix_tokens": [
                    "[CLS]"
                ],
                "suffix_tokens": [
                    "[SEP]"
                ]
            };

            super(model, utils.extend(default_config, options));
        }
    }

    BigBirdTokenizer.vocabs = [
        "spiece.model"
    ];

    tokenizers.BigBirdTokenizer = BigBirdTokenizer;
}