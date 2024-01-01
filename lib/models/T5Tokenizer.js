const utils = require("./utils");

module.exports = function (tokenizers) {
    class T5Tokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_eos_token": true,
                "eos_token": "</s>",
                "unk_token": "<unk>",
                "pad_token": "<pad>"
            };

            utils.extend(default_config, options);

            if (!default_config.additional_special_tokens) {
                default_config.additional_special_tokens = [];
                for (var i = 0; i < 100; i++)
                    default_config.additional_special_tokens.push(`<extra_id_${i}>`);
            }

            super(model, default_config);
        }
    }

    T5Tokenizer.vocabs = [
        "spiece.model"
    ];

    tokenizers.T5Tokenizer = T5Tokenizer;
}