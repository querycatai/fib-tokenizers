const fs = require("fs");
const path = require("path");
const tokenizers = require("..");

const test = require("test");
test.setup();

var models = {};

fs.readdir(path.join(__dirname, "models"))
    .filter(file => file.endsWith(".json"))
    .map(model => JSON.parse(fs.readFile(path.join(__dirname, "models", model), "utf-8")))
    .sort((a, b) => b.likes - a.likes)
    .forEach(model => {
        const _class = model.tokenizer_class.toLowerCase();
        if (!models[_class])
            models[_class] = [];
        models[_class].push(model);
    });

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

const BpeTokenizer = [
    "BloomTokenizer",
    "PreTrainedTokenizer",
    "CodeGenTokenizer",
    "GPT2Tokenizer",
    "WhisperTokenizer",
    "CLIPTokenizer",
    "BartTokenizer",
    "RobertaTokenizer",
    "Wav2Vec2CTCTokenizer",
    "BlenderbotTokenizer",
    "MarianTokenizer",
    "OpenAIGPTTokenizer",
    "M2M100Tokenizer",
    "LongformerTokenizer",
    "FLMTokenizer",
    "LEDTokenizer",
];

const BertTokenizer = [
    "BertTokenizer",
    "DistilBertTokenizer",
    "MPNetTokenizer"
];

const TikTokenizer = [
    "QWenTokenizer"
];

const base_class = {
    SentencepieceTokenizer,
    BertTokenizer,
    TikTokenizer
};

function test_model(model) {
    if (typeof model === "string")
        model = JSON.parse(fs.readFile(path.join(__dirname, "models", "models--" + model.replace(/\//g, "--") + ".json"), "utf-8"));

    if (!tokenizers.check_model(home, model.model)) {
        console.error(`Model not found: ${model.model}`);
        return;
    }

    describe(`${model.model}`, () => {
        var tokenizer;

        before(() => {
            tokenizer = tokenizers.from_folder(home, model.model);
        });

        after(() => {
            tokenizer = undefined;
        });

        it("class name", () => {
            assert.equal(tokenizer.constructor.name.toLowerCase(), model.tokenizer_class.toLowerCase());
        });

        it("special tokens", () => {
            const special_tokens = tokenizer.all_special_tokens;
            if (special_tokens)
                assert.deepEqual(special_tokens.sort(), model.special_tokens.sort());
        });

        function test_one(test) {
            describe(JSON.stringify(test.input.substr(0, 64)), () => {
                if (model.tokenizer_class != "QWenTokenizer")
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
                    assert.equal(result, test.decoded);
                });
            });
        }

        describe("datasets", () => {
            model.datasets.forEach(test_one);
            // test_one(model.datasets[model.datasets.length - 1]);
        });
    });
}

describe("tokenizer", () => {
    for (var _base_class in base_class)
        describe(_base_class, () =>
            base_class[_base_class].forEach(_class =>
                describe(_class, () =>
                    models[_class.toLowerCase()].forEach(
                        model => test_model(model)
                    )
                )
            )
        );
});

// test_model("berkeley-nest/Starling-LM-7B-alpha");
// test_model("naver-clova-ix/donut-base");

test.run(console.DEBUG);
