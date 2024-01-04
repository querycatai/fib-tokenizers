const utils = require("./utils");

module.exports = function (tokenizers) {
    class BertTokenizer extends tokenizers.BertTokenizer {
        constructor(model, options) {
            const default_config = {
                "unk_token": "[UNK]",
                "sep_token": "[SEP]",
                "pad_token": "[PAD]",
                "cls_token": "[CLS]",
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

    BertTokenizer.vocabs = [
        "vocab.txt"
    ];

    tokenizers.BertTokenizer = BertTokenizer;
}