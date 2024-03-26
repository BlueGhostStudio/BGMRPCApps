function randomStr(len) {
    var result = '';
    for (var i = 0; i < len; i++) {
        let r = Math.floor(Math.random() * 51);
        if (r < 26)
            result += String.fromCharCode(65 + r);
        else
            result += String.fromCharCode(71 + r);
    }

    return result;
}

