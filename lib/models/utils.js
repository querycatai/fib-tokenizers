var ort;
var tf;

exports.extend = function (o1, o2) {
    for (let key in o2)
        if (o2[key] !== null && o2[key] !== undefined)
            o1[key] = o2[key];

    return o1;
}

exports.tensor_wrap = function (data, opt) {
    if (Array.isArray(data) || typeof data === "string")
        return data;

    if (opt.return_tensors) {
        if (!opt.padding || !opt.truncation)
            throw new Error("ORT requires padding and truncation");

        if (opt.return_tensors === "ort") {
            if (!ort)
                ort = require("onnxruntime-common");

            const result = {};
            for (var k in data) {
                var v = data[k];
                result[k] = new ort.Tensor("int64", v.flat(), [v.length, v[0].length]);
            }

            return result;
        }

        if (opt.return_tensors === "tf") {
            if (!tf)
                tf = require("@tensorflow/tfjs-core");

            const result = {};
            for (var k in data) {
                var v = data[k];
                result[k] = tf.tensor(v, [v.length, v[0].length], "int32");
            }

            return result;
        }

        throw new Error("Unsupported return_tensors: " + opt.return_tensors);
    }

    return data;
}