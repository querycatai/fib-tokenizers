const fs = require("fs");
const path = require("path");
const tokenizers = require("..");

var models = fs.readdir(path.join(__dirname, "models"))
    .filter(file => file.endsWith(".json"))
    .map(model => JSON.parse(fs.readFile(path.join(__dirname, "models", model), "utf-8")))
    .sort((a, b) => b.likes - a.likes);

var home = path.join(process.env.HOME, ".cache/huggingface/hub");

const unknoen_models = {};
const tokenizer_list = {};

models.forEach((model) => {
    if (model.tokenizer_class) {
        var tokenizer_class = model.tokenizer_class;

        if (!tokenizer_list[tokenizer_class])
            tokenizer_list[tokenizer_class] = [];

        tokenizer_list[tokenizer_class].push(model.model);
    }
});

const sorted_tokenizer_list = {};
Object.keys(tokenizer_list).sort((a, b) => {
    var likes_a = tokenizer_list[a].reduce((acc, cur) => acc + cur.likes, 0);
    var likes_b = tokenizer_list[b].reduce((acc, cur) => acc + cur.likes, 0);

    return likes_b - likes_a;
}).forEach((key) => {
    sorted_tokenizer_list[key] = tokenizer_list[key];
});

console.log(unknoen_models);
console.log(sorted_tokenizer_list);

for (var tokenizer_class in sorted_tokenizer_list) {
    const model_path = tokenizers.resolve_model(home, sorted_tokenizer_list[tokenizer_class][0]);
    var vocab_file = "";
    fs.readdirSync(model_path).forEach((file) => {
        if (file.endsWith(".model"))
            vocab_file = file;
    });

    if (vocab_file)
        console.log(tokenizer_class, vocab_file);
}
