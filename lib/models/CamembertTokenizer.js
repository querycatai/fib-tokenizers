const utils = require("./utils");

module.exports = function (tokenizers) {
    class CamembertTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "add_bos_token": true,
                "add_eos_token": true,
                "offset": 4,
                "bos_token": "<s>",
                "eos_token": "</s>",
                "unk_token": "<unk>",
                "sep_token": "</s>",
                "pad_token": "<pad>",
                "cls_token": "<s>",
                "mask_token": "<mask>",
                "added_tokens_decoder": {
                    "0": "<s>NOTUSED",
                    "1": "<pad>",
                    "2": "</s>NOTUSED",
                    "3": "<unk>",
                    "4": "<unk>NOTUSED",
                    "5": "<s>",
                    "6": "</s>",
                }
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    CamembertTokenizer.vocabs = [
        "sentencepiece.bpe.model"
    ];

    tokenizers.CamembertTokenizer = CamembertTokenizer;
}