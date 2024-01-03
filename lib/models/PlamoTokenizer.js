module.exports = function (tokenizers) {
    class PlamoTokenizer extends tokenizers.LlamaTokenizer {
    }

    tokenizers.PlamoTokenizer = PlamoTokenizer;
}