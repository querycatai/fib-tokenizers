const utils = require("./utils");

module.exports = function (tokenizers) {
    class DistilBertTokenizer extends tokenizers.BertTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "model_input_names": [
                    "attention_mask"
                ]
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    tokenizers.DistilBertTokenizer = DistilBertTokenizer;
}