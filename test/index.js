const test = require("test");
test.setup();

const path = require("path");
const fs = require("fs");

const tokenizers = require("..");

describe("tokenizer", () => {
    describe("tiktoken", () => {
        const models = {
            cl100k_base: new tokenizers.TikTokenizer(fs.readFile(path.join(__dirname, "models/cl100k_base.tiktoken")), {
                base_model: "cl100k_base"
            }),
            p50k_base: new tokenizers.TikTokenizer(fs.readFile(path.join(__dirname, "models/p50k_base.tiktoken")), {
                base_model: "p50k_base"
            }),
            r50k_base: new tokenizers.TikTokenizer(fs.readFile(path.join(__dirname, "models/r50k_base.tiktoken")), {
                base_model: "r50k_base"
            })
        };

        function get_value(line) {
            const pos = line.indexOf(": ");
            return line.substr(pos + 2);
        }

        Object.keys(models).forEach((model) => {
            it(model, () => {
                const tokenizer = models[model];

                const lines = fs.readFileSync(path.join(__dirname, "models/TestPlans.txt"), "utf-8").split("\n");
                for (var i = 0; i < lines.length; i += 4) {
                    var line = lines[i];
                    if (line.length == 0)
                        break;
                    if (get_value(line) == model) {
                        const txt = get_value(lines[i + 1]);
                        const tok = models[model].encode(txt);
                        assert.deepEqual(tok, JSON.parse(get_value(lines[i + 2])));
                        assert.equal(models[model].decode(tok), txt);
                    }
                }
            });
        });
    });
});

test.run();
