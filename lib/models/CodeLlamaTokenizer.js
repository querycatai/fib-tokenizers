module.exports = function (tokenizers) {
    class CodeLlamaTokenizer extends tokenizers.LlamaTokenizer {
    }

    tokenizers.CodeLlamaTokenizer = CodeLlamaTokenizer;
}