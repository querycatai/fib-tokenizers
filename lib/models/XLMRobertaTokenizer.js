const utils = require("./utils");

module.exports = function (tokenizers) {
    class XLMRobertaTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_bos_token": true,
                "add_eos_token": true,
                "offset": 1,
                "added_tokens_decoder": {
                    "0": "<s>",
                    "1": "<pad>",
                    "2": "</s>",
                    "3": "<unk>",
                },
                "unk_token": "<unk>",
                "bos_token": "<s>",
                "eos_token": "</s>",
                "pad_token": "<pad>"
            };

            super(model, utils.extend(default_config, options));
        }
    }

    XLMRobertaTokenizer.vocabs = [
        "sentencepiece.bpe.model"
    ];

    tokenizers.XLMRobertaTokenizer = XLMRobertaTokenizer;
}