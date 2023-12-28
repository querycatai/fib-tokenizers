module.exports = function (tokenizers) {
    class BaichuanTokenizer extends tokenizers.LlamaTokenizer {
    }

    tokenizers.BaichuanTokenizer = BaichuanTokenizer;
}