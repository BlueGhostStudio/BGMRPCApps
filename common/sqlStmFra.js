function updateStatementFragments(fields, data, special) {
    var first = true;
    var sStm = '';
    var bindValues = {};

    for (var x in fields) {
        var fn = fields[x];
        if (data[fn] !== undefined) {
            if (!first)
                sStm += ',';
            else
                first = false;

            var bFn = ':' + fn;
            sStm += '`' + fn + '`=' + bFn;

            var spValue = false;
            if (special) spValue = special(fn, fields, data);

            if (spValue === false)
                bindValues[bFn] = data[fn];
            else
                bindValues[bFn] = spValue;
        }
    }

    return {'stm': sStm, 'bindValues': bindValues};
}

function insertStatementFragments(newData, data, special) {
    /*for (let x in data) {
        var spValue = false;
        if (special) spValue = special(x, newData, data);

        if (spValue === false)
            newData[x] = data[x];
        else
            newData[x] = spValue;
    }*/

    for (let x in newData) {
        if (data[x] !== undefined) {
            var spValue = false;
            if (special) spValue = special(x, newData, data);

            if (spValue === false)
                newData[x] = data[x];
            else
                newData[x] = spValue;
        }
    }

    var fStm = '';
    var vStm = '';
    var bindValues = {};
    var first = true;
    for (let x in newData) {
        if (!first) {
            fStm += ',';
            vStm += ',';
        } else
            first = false;

        var bFn = ':' + x;
        fStm += '`' + x + '`';
        vStm += bFn;
        bindValues[bFn] = newData[x];
    }

    return {
        'fstm': fStm, 'vstm': vStm, 'bindValues': bindValues
    }
}
