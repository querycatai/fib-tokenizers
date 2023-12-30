const fs = require("fs");
const path = require("path");
const tokenizers = require("..");

const tokenizer_tests = JSON.parse(fs.readFile(path.join(__dirname, "models", "tokenizer_tests.json"), "utf-8"));
var models = Object.keys(tokenizer_tests);

var home = path.join(process.env.HOME, ".cache/huggingface/hub");

const unknoen_models = {};
const tokenizer_list = {};

models.forEach((model) => {
    if (tokenizer_tests[model].tokenizer_class) {
        var tokenizer_class = tokenizer_tests[model].tokenizer_class;

        if (!tokenizer_list[tokenizer_class])
            tokenizer_list[tokenizer_class] = [];

        tokenizer_list[tokenizer_class].push(model);
    }
});

const sorted_tokenizer_list = {};
Object.keys(tokenizer_list).sort((a, b) => {
    var n = tokenizer_list[b].length - tokenizer_list[a].length;
    if (n !== 0)
        return n;
    return a.localeCompare(b);
}).forEach((key) => {
    sorted_tokenizer_list[key] = tokenizer_list[key];
});

console.log(unknoen_models);
console.log(sorted_tokenizer_list);

for (var tokenizer_class in sorted_tokenizer_list) {
    const model_path = tokenizers.resolve_model(home, sorted_tokenizer_list[tokenizer_class][0]);
    var vocab_file = "";
    fs.readdirSync(model_path).forEach((file) => {
        if (file.endsWith("vocab.txt"))
            vocab_file = file;
    });

    if (vocab_file)
        console.log(tokenizer_class, vocab_file);
}
