const fs = require("fs");
const path = require("path");
const tokenizers = require("..");

const test = require("test");
test.setup();

const tokenizer_tests = JSON.parse(fs.readFile(path.join(__dirname, "models", "tokenizer_tests.json"), "utf-8"));
var models = Object.keys(tokenizer_tests);

var home = path.join(process.env.HOME, ".cache/huggingface/hub");

// LlamaTokenizer tokenizer.model
// T5Tokenizer spiece.model
// XLMRobertaTokenizer sentencepiece.bpe.model
// DebertaV2Tokenizer spm.model
// CodeLlamaTokenizer tokenizer.model
// ChatGLMTokenizer tokenizer.model
// InternLMTokenizer tokenizer.model
// PegasusTokenizer spiece.model
// YiTokenizer tokenizer.model
// BaichuanTokenizer tokenizer.model
// MBart50Tokenizer sentencepiece.bpe.model
// NllbTokenizer sentencepiece.bpe.model
// CamembertTokenizer sentencepiece.bpe.model
// SpeechT5Tokenizer spm_char.model

// =================================

// M2M100Tokenizer sentencepiece.bpe.model
// SeamlessM4TTokenizer sentencepiece.bpe.model

const SentencepieceTokenizer = [
    "LlamaTokenizer",
    "T5Tokenizer",
    "XLMRobertaTokenizer",
    "DebertaV2Tokenizer",
    "CodeLlamaTokenizer",
    "ChatGLMTokenizer",
    "InternLMTokenizer",
    "PegasusTokenizer",
    "YiTokenizer",
    "BaichuanTokenizer",
    "MBart50Tokenizer",
    "NllbTokenizer",
    "CamembertTokenizer",
    "SpeechT5Tokenizer",
];

const BertTokenizer = [
    "BertTokenizer"
];

const base_class = {
    SentencepieceTokenizer,
    BertTokenizer
};

var test_limit = 1000;

describe("tokenizer", () => {
    for (var _base_class in base_class) {
        describe(_base_class, () => {
            base_class[_base_class].forEach((_class) => {
                describe(_class, () => {
                    models.forEach((model) => {
                        if (!tokenizer_tests[model].tokenizer_class)
                            return;

                        _class = _class.toLowerCase();
                        const tokenizer_class = tokenizer_tests[model].tokenizer_class.toLowerCase();
                        if (tokenizer_class !== _class)
                            return;

                        if (test_limit-- <= 0)
                            return;

                        if (!tokenizers.check_model(home, model)) {
                            console.error(`Model not found: ${model}`);
                            return;
                        }

                        describe(`${model}`, () => {
                            var tokenizer;

                            before(() => {
                                tokenizer = tokenizers.from_folder(home, model);
                            });

                            after(() => {
                                tokenizer = undefined;
                            });

                            tokenizer_tests[model].datasets.forEach((test) => {
                                describe(JSON.stringify(test.input.substr(0, 64)), () => {
                                    it("tokenize", () => {
                                        var result = tokenizer.tokenize(test.input);
                                        assert.deepEqual(result, test.tokens);
                                    });

                                    it("encode", () => {
                                        var result = tokenizer.encode(test.input);
                                        assert.deepEqual(result, test.ids);
                                    });

                                    it("decode", () => {
                                        var result = tokenizer.decode(test.ids);
                                        assert.equal(result, test.decoded_);
                                    });
                                });
                            });
                        });
                    });
                });
            });
        });
    }
});

test.run(console.DEBUG);
