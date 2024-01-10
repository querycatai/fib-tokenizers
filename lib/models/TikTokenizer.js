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
    }

    // TikTokenizer.prototype.encode_plus = utils.encode_plus;
    tokenizers.TikTokenizer = TikTokenizer;
}