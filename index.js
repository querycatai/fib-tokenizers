const fs = require("fs");
const path = require("path");

var tokenizers = require(`./addon/${path.basename(__dirname)}.node`);

require('./lib/models')(tokenizers);

const tokenizers_index = {};

for (var key in tokenizers)
    tokenizers_index[key.toLowerCase()] = tokenizers[key];

tokenizers.config_from_name = require('./lib/config_from_name');
tokenizers.from_folder = module.exports = function (home, model) {
    const model_home = path.join(home, "models--" + model.replace(/\//g, "--"));
    var model_path;
    if (fs.existsSync(path.join(model_home, "refs")) && path.join(model_home, "snapshots")) {
        const tag = fs.readFileSync(path.join(model_home, "refs", "main"), "utf-8");
        model_path = path.join(model_home, "snapshots", tag);
    } else
        model_path = model_home;

    function get_json(file) {
        if (fs.existsSync(file))
            return JSON.parse(fs.readFileSync(file, "utf-8"));

        return {};
    }

    var config = get_json(path.join(model_path, "config.json"));
    var tokenizer_config = get_json(path.join(model_path, "tokenizer_config.json"));
    var special_tokens_map = get_json(path.join(model_path, "special_tokens_map.json"));
    var file_list = fs.readdirSync(model_path);

    const model_config = tokenizers.config_from_name(config, tokenizer_config, file_list, special_tokens_map);

    const tokenizer_class = tokenizers_index[model_config.tokenizer_class.toLowerCase()];
    const vocabs = tokenizer_class.vocabs.map(vocab => fs.readFileSync(path.join(model_path, vocab)));

    var tokenizer;
    switch (vocabs.length) {
        case 1:
            tokenizer = new tokenizer_class(vocabs[0], model_config);
            break;
        case 2:
            tokenizer = new tokenizer_class(vocabs[0], vocabs[1], model_config);
            break;
        default:
            throw new Error("Unsupported vocabs");
    }
    tokenizer.config = model_config;

    return tokenizer;
}

module.exports = tokenizers;
