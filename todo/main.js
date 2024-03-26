methods = [
    "join",
    "list",
    "newRecord",
    "removeAllCompleted",
    "remove",
    "content",
    "priority",
    "label",
    "state"
];

var DB;
function constructor() {
    JS.loadModule('jsDB');
    JS.loadModule('jsFile');
    
    var file = new JsFile;
    if (!file.exists(JS.__PATH_DATA__ + '/todo.db'))
        file.copy(JS.__PWD__ + '/todo.db', JS.__PATH_DATA__ + '/todo.db');
    
    DB = new JsDB;
    DB.openDB("todo.db");
}

function _selectItem (id) {
    let result = DB.exec('SELECT * FROM `todo` WHERE `id`=:I', {
        ":I": id
    });

    if (result.ok) {
        if (result.rows.length > 0 )
            return result.rows[0];
        else
            return false;
    } else
        return false;
}

function _getCallerToken(caller) {
    let token = JS.call(caller, 'account', 'getToken', [])[0];

    return token != null && token.length > 0 ? token : false;
}

function _updateField(caller, id, field, value) {
    let token = _getCallerToken(caller);
    if (token != false) {
        let result = DB.exec('UPDATE `todo` SET `' + field + '`=:V WHERE `id`=:I AND `own`=:T', {
            ':V': value,
            ':I': id,
            ':T': token
        });

        return result.ok;
    } else
        return false;
}

function join(caller) {
    JS.addRelClient(caller);

    return caller.__ID__;
}

function list(caller, lid) {
    let result = DB.exec('SELECT * FROM `todo` WHERE `lid`=:L ORDER BY `state`, `date` DESC, `PRI`', {
        ':L': lid
    });

    if (result.ok)
        return [result.rows];
    else
        return false;
}

function newRecord(caller, lid, cnt) {
    let token = _getCallerToken(caller);

    if (token != false) {
        let result = DB.exec('INSERT INTO `todo` (`lid`, `content`, `own`) VALUES (:L, :C, :T)', {
            ':L': lid,
            ':C': cnt,
            ':T': token
        });

        if (result.ok) {
            let item = _selectItem(result.lastInsert);
            JS.emitSignal("newRecord", [item]);

            return item.row;
        } else
            return false;
    } else
        return false;
}

function removeAllCompleted(caller, lid) {
    let token = _getCallerToken(caller);

    if (token != false) {
        let removedTodoRows = DB.exec('SELECT id FROM `todo` WHERE `lid`=:L AND `state`=1', {
            ':L': lid
            }).rows;
        let result =  DB.exec('DELETE FROM `todo` WHERE `lid`=:L AND `state`=1', {
            ':L': lid
        });

        if (result.ok) {
            let removedTodos = [];
            for (var x in removedTodoRows)
                removedTodos.push(removedTodoRows[x].id);

            JS.emitSignal("allCompletedRemoved", [lid, removedTodos]);

            return true;
        } else
            return false;
    } else
        return false;
}

function remove(caller, id) {
    let token = _getCallerToken(caller);

    if (token != false) {
        let result =  DB.exec('DELETE FROM `todo` WHERE `id`=:I', {
            ':I': id
        });

        if (result.ok) {
            JS.emitSignal("removed", [id]);
            return true;
        } else
            return false;
    } else
        return false;
}

function content(caller, id, cnt) {
    if (cnt == undefined) {
        let result = _selectItem(id);

        if (result != false)
            return result.content;
        else
            return false;
    } else {
        if (_updateField(caller, id, "content", cnt)) {
            let cnt = content(caller, id);
            JS.emitSignal("contentUpdated", [id, cnt]);

            return content(caller, id);
        } else
            return false;
    }
}

function priority(caller, id, pri) {
    if (pri == undefined) {
        let result = _selectItem(id);

        if (result != false)
            return result.PRI;
        else
            return false;
    } else {
        if (_updateField(caller, id, "PRI", pri)) {
            let pri = priority(caller, id);
            JS.emitSignal("priorityUpdated", [id, pri]);

            return priority(caller, id);
        } else
            return false;
    }
}

function label(caller, id, lab) {
    if (lab == undefined) {
        let result = _selectItem(id);

        if (result != false)
            return result.label;
        else
            return false;
    } else {
        if (_updateField(caller, id, "label", lab)) {
            let lab = label(caller, id);
            JS.emitSignal("labelUpdated", [id, lab]);

            return label(caller, id);
        } else
            return false;
    }
}

function state(caller, id, st) {
    if (st == undefined) {
        let result = _selectItem(id);

        if (result != false)
            return result.state;
        else
            return false;
    } else {
        if (_updateField(caller, id, "state", st)) {
            let st = state(caller, id);
            JS.emitSignal("stateUpdated", [id, st]);

            return state(caller, id);
        } else
            return false;
    }
}

