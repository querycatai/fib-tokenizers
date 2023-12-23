const path = require('path');
var tokenizers = require(`./addon/${path.basename(__dirname)}.node`);

module.exports = tokenizers;
