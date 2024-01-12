const utils = require("./utils");

module.exports = function (tokenizers) {
    class TikTokenizer extends tokenizers.TikTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "model_input_names": [
                    "token_type_ids",
                    "attention_mask"
                ]
            };

            super(vocab, utils.extend(default_config, options));
        }

        encode_plus(text, text_pair, opt) {
            return utils.tensor_wrap(super.encode_plus(text, text_pair, opt), arguments[arguments.length - 1]);
        }
    }

    // TikTokenizer.prototype.encode_plus = utils.encode_plus;
    tokenizers.TikTokenizer = TikTokenizer;
}