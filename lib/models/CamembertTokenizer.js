const utils = require("./utils");

module.exports = function (tokenizers) {
    class CamembertTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_bos_token": true,
                "add_eos_token": true,
                "offset": 4,
                "added_tokens_decoder": {
                    "0": "<s>NOTUSED",
                    "1": "<pad>",
                    "2": "</s>NOTUSED",
                    "3": "<unk>",
                    "4": "</unk>NOTUSED",
                    "5": "<s>",
                    "6": "</s>",
                }
            };

            super(model, utils.extend(default_config, options));
        }
    }

    CamembertTokenizer.vocabs = [
        "sentencepiece.bpe.model"
    ];

    tokenizers.CamembertTokenizer = CamembertTokenizer;
}