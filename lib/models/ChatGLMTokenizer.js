const utils = require("./utils");

module.exports = function (tokenizers) {
    class ChatGLMTokenizer extends tokenizers.SentencepieceTokenizer {
        constructor(model, options) {
            const default_config = {
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

            super(model, utils.extend(default_config, options));
        }
    }

    ChatGLMTokenizer.vocabs = [
        "tokenizer.model"
    ];

    tokenizers.ChatGLMTokenizer = ChatGLMTokenizer;
}