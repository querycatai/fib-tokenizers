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

const SentencepieceTokenizer = [
    "LlamaTokenizer",
    "ChatGLMTokenizer",
    "T5Tokenizer",
    "XLMRobertaTokenizer",
    "SpeechT5Tokenizer",
    "SeamlessM4TTokenizer",
    "CodeLlamaTokenizer",
    "NllbTokenizer",
    "YiTokenizer",
    "DebertaV2Tokenizer",
    "PegasusTokenizer",
    "InternLMTokenizer",
    "MBart50Tokenizer",
    "CamembertTokenizer",
    "PlamoTokenizer",
    "MBartTokenizer",
    "SkyworkTokenizer",
    "XGLMTokenizer",
    "BigBirdTokenizer",

    // "XLNetTokenizer",
    // "BaichuanTokenizer", // custom implementation
    // "Midm_bitext_Tokenizer", // custom implementation
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
    // BertTokenizer,
    TikTokenizer
};

function fix_text(text) {
    text = text.replace(/^\s+|\s+$/g, "")
        .replace(/\s+/g, " ")
        .replace(/\s+\./g, ".")
        .replace(/\s+!/g, "!");

    return text;
    return text.replace(/<\/s>|<s>|<unk>|\[UNK\]|<pad>|\r|\n|\t/g, " ")
        .replace(/ +/g, " ")
        .replace(/^ +| +$/g, "")
        .replace(/ +\./g, ".")
        .replace(/ +!/g, "!");
}

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
            // console.log(model.tokenizer_class);
        });

        // it("special tokens", () => {
        //     const special_tokens = tokenizer.all_special_tokens;
        //     if (special_tokens)
        //         assert.deepEqual(special_tokens.sort(), model.special_tokens.sort());
        // });

        function test_one(test) {
            describe(JSON.stringify(test.input.substr(0, 64)), () => {
                it("tokenize", () => {
                    var result = tokenizer.tokenize(test.input);
                    result = result.map(token => token.replace(/�/g, ""));
                    assert.deepEqual(result, test.tokens);
                    // console.log(result);
                });

                it("encode", () => {
                    var result = tokenizer.encode(test.input);
                    assert.deepEqual(result, test.ids);
                });

                it("decode", () => {
                    var result = tokenizer.decode(test.ids);
                    assert.equal(fix_text(result), fix_text(test.decoded));
                });
            });
        }

        describe("datasets", () => {
            model.datasets.forEach(test_one);
            // test_one(model.datasets[model.datasets.length - 1]);
        });
    });
}

function test_tokenizer(tokenizer_class) {
    describe(tokenizer_class, () => models[tokenizer_class.toLowerCase()].forEach(test_model));
}

describe("tokenizer", () => {
    for (var _base_class in base_class)
        describe(_base_class, () => base_class[_base_class].forEach(test_tokenizer));
});

// test_model("codellama/CodeLlama-13b-hf");
// test_tokenizer("QWenTokenizer");

test.run();
// test.run(console.DEBUG);
