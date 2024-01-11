const utils = require("./utils");

module.exports = function (tokenizers) {
    class T5Tokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "add_eos_token": true,
                "add_eos_if_not_present": true,
                "eos_token": "</s>",
                "unk_token": "<unk>",
                "pad_token": "<pad>"
            };

            default_config.additional_special_tokens = [];
            for (var i = 0; i < 100; i++)
                default_config.additional_special_tokens.push(`<extra_id_${i}>`);

            utils.extend(default_config, options);

            default_config.additional_special_tokens = default_config.additional_special_tokens.reverse();

            for (var i = 0; i < 100; i++)
                if (!default_config.additional_special_tokens.includes(`<extra_id_${i}>`))
                    default_config.additional_special_tokens.push(`<extra_id_${i}>`);

            super(vocab, default_config);
        }
    }

    T5Tokenizer.vocabs = [
        "spiece.model"
    ];

    tokenizers.T5Tokenizer = T5Tokenizer;
}