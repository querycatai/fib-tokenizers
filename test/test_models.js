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
    "BaichuanTokenizer",

    // "XLNetTokenizer",
    // "Midm_bitext_Tokenizer", // custom implementation
];

const BpeTokenizer = [
    "GPT2Tokenizer",
    "CodeGenTokenizer",
    "WhisperTokenizer",
    "RobertaTokenizer",
    // ====================================
    // "CLIPTokenizer",
    // "BartTokenizer",
    // "Wav2Vec2CTCTokenizer",
    // "BlenderbotTokenizer",
    // "MarianTokenizer",
    // "OpenAIGPTTokenizer",
    // "M2M100Tokenizer",
    // "LongformerTokenizer",
    // "FLMTokenizer",
    // "LEDTokenizer",
    // "DebertaTokenizer",
    // "GLMGPT2Tokenizer",
    // "BlenderbotSmallTokenizer",
    // "VitsTokenizer",
];

const BertTokenizer = [
    "BertTokenizer",
    "DistilBertTokenizer",
    "MPNetTokenizer",
    "ElectraTokenizer"
];

const TikTokenizer = [
    "QWenTokenizer"
];

const FastTokenizer = [
    // "BloomTokenizer",
    // "PreTrainedTokenizer",
];

const base_class = {
    SentencepieceTokenizer,
    BpeTokenizer,
    BertTokenizer,
    TikTokenizer,
    FastTokenizer
};

function fix_text(text) {
    text = text
        .replace(/<s>|<\/s>|<pad>/g, "")
        .replace(/^\s+|\s+$/g, "")
        .replace(/\s*'\s*/g, "'")
        .replace(/\s+/g, " ")
        .replace(/\s+!/g, "!")
        .replace(/\s*\.\s*/g, ".")

    return text;
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
            // fs.writeFile(path.join(__dirname, "models", "models--" + model.model.replace(/\//g, "--") + ".json"), JSON.stringify(model, null, 4));
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
            it(`tokenize - ${JSON.stringify(test.input.substr(0, 64))}`, () => {
                var result = tokenizer.tokenize(test.input);
                result = result.map(token => token.replace(/�/g, ""));
                // test.tokens = result;
                assert.deepEqual(result, test.tokens);
            });

            it(`encode - ${JSON.stringify(test.input.substr(0, 64))}`, () => {
                var result = tokenizer.encode(test.input);
                // test.ids = result;
                assert.deepEqual(result, test.ids);
            });

            it(`decode - ${JSON.stringify(test.input.substr(0, 64))}`, () => {
                var result = tokenizer.decode(test.ids);
                // test.decoded = result;
                assert.equal(result, test.decoded);
            });
        }

        model.datasets.forEach(test_one);
        // test_one(model.datasets[model.datasets.length - 1]);
    });
}

function test_tokenizer(tokenizer_class) {
    describe(tokenizer_class, () => models[tokenizer_class.toLowerCase()].forEach(test_model));
}

for (var _base_class in base_class)
    describe(_base_class, () => base_class[_base_class].forEach(test_tokenizer));

// test_tokenizer("RobertaTokenizer");
// test_model("impira/layoutlm-invoices");

test.run();
// test.run(console.DEBUG);
