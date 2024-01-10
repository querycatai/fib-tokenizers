const utils = require("./utils");

module.exports = function (tokenizers) {
    class PreTrainedTokenizerFast extends tokenizers.FastTokenizer {
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

    tokenizers.PreTrainedTokenizerFast = PreTrainedTokenizerFast;
}