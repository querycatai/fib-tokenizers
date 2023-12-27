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
            Object.assign(default_config, options);
            super(model, default_config);
        }
    }

    ChatGLMTokenizer.vocabs = [
        "tokenizer.model"
    ];

    tokenizers.ChatGLMTokenizer = ChatGLMTokenizer;
}