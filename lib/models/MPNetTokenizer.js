const utils = require("./utils");

module.exports = function (tokenizers) {
    class MPNetTokenizer extends tokenizers.BertTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_bos_token": true,
                "add_eos_token": true,
                "bos_token": "<s>",
                "eos_token": "</s>",
                "unk_token": "[UNK]",
                "sep_token": "</s>",
                "pad_token": "<pad>",
                "cls_token": "<s>",
                "mask_token": "<mask>",
                "additional_special_tokens": [
                    "<unk>"
                ],
                "prefix_tokens": [],
                "suffix_tokens": []
            };

            super(model, utils.extend(default_config, options));
        }
    }

    tokenizers.MPNetTokenizer = MPNetTokenizer;
}