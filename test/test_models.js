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
// ChatGLMTokenizer ice_text.model
// DebertaV2Tokenizer spm.model
// CodeLlamaTokenizer tokenizer.model
// BaichuanTokenizer tokenizer.model
// InternLMTokenizer tokenizer.model
// PegasusTokenizer spiece.model
// YiTokenizer tokenizer.model

// =================================

// MBart50Tokenizer sentencepiece.bpe.model
// ReplitLMTokenizer spiece.model
// CamembertTokenizer sentencepiece.bpe.model
// GLMChineseTokenizer cog-pretrain.model
// M2M100Tokenizer sentencepiece.bpe.model
// SpeechT5Tokenizer spm_char.model

// =================================

// NllbTokenizer sentencepiece.bpe.model

const test_class = [
    "LlamaTokenizer",
    "T5Tokenizer",
    // =================================
    "CodeLlamaTokenizer",
    "DebertaV2Tokenizer",
    "ChatGLMTokenizer",
    "BaichuanTokenizer",
    "XLMRobertaTokenizer",
    "InternLMTokenizer",
    "PegasusTokenizer",
    "YiTokenizer",
];

var test_limit = 1000;
describe("tokenizer", () => {
    test_class.forEach((_class) => {
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
                        tokenizer = null;
                    });

                    tokenizer_tests[model].datasets.forEach((test) => {
                        describe(JSON.stringify(test.input.substr(0, 64)), () => {
                            it("encode", () => {
                                const result = tokenizer.encode(test.input);
                                assert.deepEqual(result, test.ids);
                            });

                            it("tokenize", () => {
                                const result = tokenizer.tokenize(test.input);
                                assert.deepEqual(result, test.tokens);
                            });

                            // it("decode", () => {
                            //     const result = tokenizer.decode(test.ids);
                            //     assert.equal(result, test.decoded_);
                            // });
                        });
                    });
                });
            });
        });
    });
});

test.run(console.DEBUG);
