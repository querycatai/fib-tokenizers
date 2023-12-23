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

    describe("tiktoken", () => {
        const models = {
            cl100k_base: new tokenizers.TikTokenizer(fs.readFile(path.join(__dirname, "models/cl100k_base.tiktoken"), "utf-8").split("\n"), {
                base_model: "cl100k_base"
            }),
            p50k_base: new tokenizers.TikTokenizer(fs.readFile(path.join(__dirname, "models/p50k_base.tiktoken"), "utf-8").split("\n"), {
                base_model: "p50k_base"
            }),
            r50k_base: new tokenizers.TikTokenizer(fs.readFile(path.join(__dirname, "models/r50k_base.tiktoken"), "utf-8").split("\n"), {
                base_model: "r50k_base"
            })
        };

        function get_value(line) {
            const pos = line.indexOf(": ");
            return line.substr(pos + 2);
        }

        Object.keys(models).forEach((model) => {
            it(model, () => {
                const tokenizer = models[model];

                const lines = fs.readFileSync(path.join(__dirname, "models/TestPlans.txt"), "utf-8").split("\n");
                for (var i = 0; i < lines.length; i += 4) {
                    var line = lines[i];
                    if (line.length == 0)
                        break;
                    if (get_value(line) == model) {
                        const txt = get_value(lines[i + 1]);
                        const tok = models[model].encode(txt);
                        assert.deepEqual(tok, JSON.parse(get_value(lines[i + 2])));
                        assert.equal(models[model].decode(tok), txt);
                    }
                }
            });
        });
    });
});

test.run();
