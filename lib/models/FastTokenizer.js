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

        encode_plus(text, text_pair, opt) {
            return utils.tensor_wrap(super.encode_plus(text, text_pair, opt), arguments[arguments.length - 1]);
        }
    }

    // FastTokenizer.prototype.encode_plus = utils.encode_plus;
    FastTokenizer.vocabs = [
        "tokenizer.json"
    ];

    tokenizers.FastTokenizer = FastTokenizer;
}