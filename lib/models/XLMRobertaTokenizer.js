module.exports = function (tokenizers) {
    class XLMRobertaTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
                "add_bos_token": true,
                "offset": 1
            };
            Object.assign(default_config, options);
            super(model, default_config);
        }
    }

    XLMRobertaTokenizer.vocabs = [
        "sentencepiece.bpe.model"
    ];

    tokenizers.XLMRobertaTokenizer = XLMRobertaTokenizer;
}