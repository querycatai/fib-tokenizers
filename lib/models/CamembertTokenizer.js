const utils = require("./utils");

module.exports = function (tokenizers) {
    class CamembertTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_bos_token": true,
                "add_eos_token": true,
                "offset": 4,
                "added_tokens_decoder": {
                    "3": "<unk>"
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