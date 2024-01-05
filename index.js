const fs = require("fs");
const path = require("path");

var tokenizers = require(`./addon/${path.basename(__dirname)}.node`);
require('./lib/models')(tokenizers);

const tokenizers_index = {};
for (var key in tokenizers)
    tokenizers_index[key.toLowerCase()] = tokenizers[key];

tokenizers.resolve_model = require('./lib/resolve_model');
tokenizers.config_from_name = require('./lib/config_from_name');

function get_json(file) {
    if (fs.existsSync(file))
        return JSON.parse(fs.readFileSync(file, "utf-8"));
}

tokenizers.from_folder = function (home, model) {
    const model_path = tokenizers.resolve_model(home, model);

    const model_config = tokenizers.config_from_name(
        get_json(path.join(model_path, "config.json")),
        get_json(path.join(model_path, "tokenizer_config.json")),
        fs.readdirSync(model_path)
    );

    const added_tokens = get_json(path.join(model_path, "added_tokens.json"));
    if (added_tokens)
        model_config.added_tokens = added_tokens;

    const special_tokens_map = get_json(path.join(model_path, "special_tokens_map.json"));
    if (special_tokens_map) {
        if (special_tokens_map.additional_special_tokens && model_config.additional_special_tokens) {
            special_tokens_map.additional_special_tokens.forEach(token => {
                if (!model_config.additional_special_tokens.includes(token))
                    model_config.additional_special_tokens.push(token);
            });
            delete special_tokens_map.additional_special_tokens;
        }

        Object.assign(model_config, special_tokens_map);

    }

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

tokenizers.check_model = function (home, model) {
    const model_path = tokenizers.resolve_model(home, model);

    const model_config = tokenizers.config_from_name(
        get_json(path.join(model_path, "config.json")),
        get_json(path.join(model_path, "tokenizer_config.json"))
    );

    const tokenizer_class = tokenizers_index[model_config.tokenizer_class.toLowerCase()];
    return tokenizer_class.vocabs.every(vocab => fs.existsSync(path.join(model_path, vocab)));
}

module.exports = tokenizers;
