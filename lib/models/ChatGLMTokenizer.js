const utils = require("./utils");

module.exports = function (tokenizers) {
    class ChatGLMTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(vocab, options) {
            const default_config = {
                "padding_side": "left",
                "additional_special_tokens": [
                    "[MASK]",
                    "[gMASK]",
                    "[sMASK]",
                    "sop",
                    "eop"
                ],
                "prefix_tokens": [
                    "[gMASK]",
                    "sop"
                ]
            };

            super(vocab, utils.extend(default_config, options));
        }
    }

    ChatGLMTokenizer.vocabs = [
        "tokenizer.model"
    ];

    tokenizers.ChatGLMTokenizer = ChatGLMTokenizer;
}