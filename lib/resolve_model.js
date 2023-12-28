const fs = require("fs");
const path = require("path");

module.exports = function (home, model) {
    const model_home = path.join(home, "models--" + model.replace(/\//g, "--"));
    if (fs.existsSync(path.join(model_home, "refs")) && path.join(model_home, "snapshots")) {
        const tag = fs.readFileSync(path.join(model_home, "refs", "main"), "utf-8");
        return path.join(model_home, "snapshots", tag);
    } else
        return model_home;
}
