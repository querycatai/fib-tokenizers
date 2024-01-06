const utils = require("./utils");

module.exports = function (tokenizers) {
    class PreTrainedTokenizerFast extends tokenizers.BpeTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "unk_token": "<unk>",
                "bos_token": "<s>",
                "eos_token": "</s>",
                "pad_token": "<pad>"
            };

            utils.extend(default_config, options)

            vocab = JSON.parse(vocab.toString());
            var vocab_data = Buffer.from(JSON.stringify(vocab.model.vocab));
            var merges_data = Buffer.from(vocab.model.merges.join("\n"));
            delete vocab.model;

            utils.extend(default_config, vocab);

            super(vocab_data, merges_data, default_config);
        }
    }

    PreTrainedTokenizerFast.vocabs = [
        "tokenizer.json"
    ];

    tokenizers.PreTrainedTokenizerFast = PreTrainedTokenizerFast;
}