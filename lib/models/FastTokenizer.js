const utils = require("./utils");

module.exports = function (tokenizers) {
    class FastTokenizer extends tokenizers.FastTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "unk_token": "<unk>",
                "bos_token": "<s>",
                "eos_token": "</s>",
                "pad_token": "<pad>"
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    FastTokenizer.vocabs = [
        "tokenizer.json"
    ];

    tokenizers.FastTokenizer = FastTokenizer;
}