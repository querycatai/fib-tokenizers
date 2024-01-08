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
    "CLIPTokenizer",
    "BartTokenizer",
    "BlenderbotTokenizer",
    "OpenAIGPTTokenizer",
    "LongformerTokenizer",
    "FLMTokenizer",
    "LEDTokenizer",
    "GLMGPT2Tokenizer",

    // ====================================
    // "Wav2Vec2CTCTokenizer",
    // "MarianTokenizer",
    // "M2M100Tokenizer",
    // "DebertaTokenizer",
    // "BlenderbotSmallTokenizer",
    // "VitsTokenizer",
    // ====================================
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
    "BloomTokenizerFast",
    "PreTrainedTokenizerFast",
    "GPT2TokenizerFast",
    "LlamaTokenizerFast"
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
            if (update_data)
                fs.writeFile(path.join(__dirname, "models", "models--" + model.model.replace(/\//g, "--") + ".json"), JSON.stringify(model, null, 4));
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

        it("encode", () => {
            var res1 = tokenizer.encode("a b c d e f g h i j k l m n o p");
            var res2 = tokenizer.encode("a b c d e f g h i j k l m n o p", {
                max_length: 100
            });

            assert.deepEqual(res1, res2);

            var res3 = tokenizer.encode("a b c d e f g h i j k l m n o p", {
                max_length: 3
            });

            assert.equal(res3.length, 3);
        });

        describe("encode_plus", () => {
            it("batch_encode", () => {
                var res1 = tokenizer.encode_plus(["a a", "b b b b b b b b b b b b b b b b b"], {
                    truncation: true,
                    max_length: 10,
                    padding: model.pad_token !== undefined
                });

                if (update_data) {
                    model.batch_encode.input_ids = res1.input_ids;
                    model.batch_encode.attention_mask = res1.attention_mask;
                }

                assert.deepEqual(res1.input_ids, model.batch_encode.input_ids);
                assert.deepEqual(res1.attention_mask, model.batch_encode.attention_mask);
            });

            it("pair_encode", () => {
                var res1 = tokenizer.encode_plus("a a", "b b b b");

                if (update_data) {
                    model.pair_encode.input_ids = res1.input_ids;
                    if (model.pair_encode.token_type_ids)
                        model.pair_encode.token_type_ids = res1.token_type_ids;
                }

                assert.deepEqual(res1.input_ids, model.pair_encode.input_ids);
                if (model.pair_encode.token_type_ids)
                    assert.deepEqual(res1.token_type_ids, model.pair_encode.token_type_ids);
            });
        });

        function test_one(test) {
            it(`tokenize - ${JSON.stringify(test.input.substr(0, 64))}`, () => {
                var result = tokenizer.tokenize(test.input);
                result = result.map(token => token.replace(/�/g, ""));
                if (update_data)
                    test.tokens = result;
                assert.deepEqual(result, test.tokens);
            });

            it(`encode - ${JSON.stringify(test.input.substr(0, 64))}`, () => {
                var result = tokenizer.encode(test.input);
                if (update_data)
                    test.ids = result;
                assert.deepEqual(result, test.ids);
            });

            it(`decode - ${JSON.stringify(test.input.substr(0, 64))}`, () => {
                var result = tokenizer.decode(test.ids);
                if (update_data)
                    test.decoded = result;
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

const update_data = false;

for (var _base_class in base_class)
    describe(_base_class, () => base_class[_base_class].forEach(test_tokenizer));

// test_tokenizer("LlamaTokenizer");
// test_model("xverse/XVERSE-13B");

// test.run();
test.run(console.DEBUG);
