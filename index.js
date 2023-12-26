const path = require('path');
var tokenizers = require(`./addon/${path.basename(__dirname)}.node`);

tokenizers.config_from_name = require('./lib/config_from_name');

module.exports = tokenizers;
