const path = require('path');
var tokenizers = require(`./addon/${path.basename(__dirname)}.node`);

tokenizers.tokenizer_class_from_name = require('./lib/tokenizer_class_from_name');

module.exports = tokenizers;
