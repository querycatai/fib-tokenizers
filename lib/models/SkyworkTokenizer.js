module.exports = function (tokenizers) {
    class SkyworkTokenizer extends tokenizers.LlamaTokenizer {
    }

    tokenizers.SkyworkTokenizer = SkyworkTokenizer;
}