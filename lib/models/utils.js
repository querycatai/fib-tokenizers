exports.extend = function (o1, o2) {
    for (let key in o2)
        if (o2[key] !== null && o2[key] !== undefined)
            o1[key] = o2[key];

    return o1;
}