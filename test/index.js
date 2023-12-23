const test = require("test");
test.setup();

const path = require("path");
const fs = require("fs");

const tokenizers = require("..");

describe("tokenizer", () => {
    describe("BasicTokenizer", () => {
        it("chinese", () => {
            const tokenizer = new tokenizers.BasicTokenizer();
            assert.deepEqual(tokenizer.tokenize("ah\u535A\u63A8zz"), ["ah", "\u535A", "\u63A8", "zz"])
        });

        it("lower", () => {
            const tokenizer = new tokenizers.BasicTokenizer({
                do_lower_case: true
            });

            assert.deepEqual(tokenizer.tokenize(" \tHeLLo!how  \n Are yoU?  "), ["hello", "!", "how", "are", "you", "?"]);
            assert.deepEqual(tokenizer.tokenize("H\u00E9llo"), ["hello"]);
        });

        it("lower_strip_accents_false", () => {
            const tokenizer = new tokenizers.BasicTokenizer({
                do_lower_case: true,
                strip_accents: false
            });

            assert.deepEqual(tokenizer.tokenize(" \tHäLLo!how  \n Are yoU?  "), ["hällo", "!", "how", "are", "you", "?"]);
            assert.deepEqual(tokenizer.tokenize("H\u00E9llo"), ["h\u00E9llo"]);
        });

        it("lower_strip_accents_true", () => {
            const tokenizer = new tokenizers.BasicTokenizer({
                do_lower_case: true,
                strip_accents: true
            });

            assert.deepEqual(tokenizer.tokenize(" \tHäLLo!how  \n Are yoU?  "), ["hallo", "!", "how", "are", "you", "?"]);
            assert.deepEqual(tokenizer.tokenize("H\u00E9llo"), ["hello"]);
        });

        it("lower_strip_accents_default", () => {
            const tokenizer = new tokenizers.BasicTokenizer({
                do_lower_case: true
            });

            assert.deepEqual(tokenizer.tokenize(" \tHäLLo!how  \n Are yoU?  "), ["hallo", "!", "how", "are", "you", "?"]);
            assert.deepEqual(tokenizer.tokenize("H\u00E9llo"), ["hello"]);
        });

        it("no_lower", () => {
            const tokenizer = new tokenizers.BasicTokenizer({
                do_lower_case: false
            });

            assert.deepEqual(tokenizer.tokenize(" \tHeLLo!how  \n Are yoU?  "), ["HeLLo", "!", "how", "Are", "yoU", "?"]);
        });

        it("no_lower_strip_accents_false", () => {
            const tokenizer = new tokenizers.BasicTokenizer({
                do_lower_case: false,
                strip_accents: false
            });

            assert.deepEqual(tokenizer.tokenize(" \tHäLLo!how  \n Are yoU?  "), ["HäLLo", "!", "how", "Are", "yoU", "?"]);
        });

        it("no_lower_strip_accents_true", () => {
            const tokenizer = new tokenizers.BasicTokenizer({
                do_lower_case: false,
                strip_accents: true
            });

            assert.deepEqual(tokenizer.tokenize(" \tHäLLo!how  \n Are yoU?  "), ["HaLLo", "!", "how", "Are", "yoU", "?"]);
        });

        it("splits_on_punctuation", () => {
            const tokenizer = new tokenizers.BasicTokenizer()
            text = "a\n'll !!to?'d of, can't."
            expected = ["a", "'", "ll", "!", "!", "to", "?", "'", "d", "of", ",", "can", "'", "t", "."]
            assert.deepEqual(tokenizer.tokenize(text), expected);
        });
    });

    it("WordpieceTokenizer", () => {
        const vocab = ["[UNK]", "[CLS]", "[SEP]", "want", "##want", "##ed", "wa", "un", "runn", "##ing"];
        const tokenizer = new tokenizers.WordpieceTokenizer(vocab, {
            unk_token: "[UNK]"
        });

        assert.deepEqual(tokenizer.tokenize(""), []);
        assert.deepEqual(tokenizer.tokenize("unwanted running"), ["un", "##want", "##ed", "runn", "##ing"]);
        assert.deepEqual(tokenizer.tokenize("unwantedX running"), ["[UNK]", "runn", "##ing"]);
    });
});

test.run();
