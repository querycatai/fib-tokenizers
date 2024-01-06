const utils = require("./utils");

module.exports = function (tokenizers) {
    class BlenderbotTokenizer extends tokenizers.BartTokenizer {
        constructor(vocab, mergs, options) {
            const default_config = {
                "add_bos_token": false
            };

            super(vocab, mergs, utils.extend(default_config, options));
        }
    }

    tokenizers.BlenderbotTokenizer = BlenderbotTokenizer;
}