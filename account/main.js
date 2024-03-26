var DB;

//methods = ['register', 'updateAccount', 'addAccessPermissions', 'login', 'logout', 'getToken', 'accountList'];

methods = [
    { "name": "register", "desc": "Register new Account" },
    { "name": "updateAccount", "desc": "Update Account" },
    { "name": "addAccessPermissions", "desc": "Add Access permissions" },
    { "name": "login", "desc": "Login" },
    { "name": "logout", "desc": "Logout" },
    { "name": "getToken", "desc": "Get Access Token" },
    { "name": "accountList", "desc": "List Account" }
]

var loginedCallers = [];

function constructor() {
    JS.loadModule('jsDB');
    JS.loadModule('jsFile');
    JS.include('../common/randomStr.js');
    JS.include('../common/sqlStmFra.js');

    var file = new JsFile;
    if (!file.exists(JS.__PATH_DATA__)) {
        file.mkpath(JS.__PATH_DATA__);
        file.copy(JS.__PATH_APP__ + '/account.db', JS.__PATH_DATA__ + '/account.db');
    }

    DB = new JsDB;
    DB.openDB('account.db');

    JS.onRelClientRemoved((caller) => {
        //console.log("----------rel client removed----------", caller.__ID__);
        logout(caller);
    });
}

function register(caller) {
    var rs = randomStr(6);
    if (DB.exec('SELECT * FROM `account` WHERE `token`=:T', {
              ':T': rs
          }).rows.length == 0) {
        DB.exec('INSERT INTO `account` (`token`) VALUES (:T)', {':T': rs});
        // caller.setPrivateData(JS, 'token', rs);

        return rs;
    } else
        return register(caller);
}

function updateAccount(caller, token, account) {
    var stmFra = updateStatementFragments(['nick'], {
        'nick': account
    });
    stmFra.bindValues[':T'] = token;
    return DB
        .exec(
            'UPDATE `account` SET ' + stmFra.stm + ' WHERE `token`=:T',
            stmFra.bindValues)
        .ok;
}

function addAccessPermissions(caller, token, app, grp, obj, attribute) {
    if (!grp)
        grp = '';
    if (!obj)
        obj = '*';

    attribute = JSON.stringify(attribute);

    if (DB.exec('SELECT * FROM `access` WHERE `token`=:T AND `app`=:A AND `grp`=IFNULL(:G, "") AND `obj`=:O', {
        ':T': token,
        ':A': app,
        ':G': grp,
        ':O': obj
    }).rows.length == 0)
        return DB.exec('INSERT INTO `access` (`token`, `app`, `grp`, `obj`, `attribute`) VALUES (:T, :A, IFNULL(:G, ""), :O, :ATTR)', {
            ':T': token,
            ':A': app,
            ':G': grp,
            ':O': obj,
            ':ATTR': attribute
        }).ok;
    else {
        return DB.exec('UPDATE `access` SET `attribute`=:ATTR WHERE `token`=:T AND `app`=:A AND `grp`=IFNULL(:G, "") AND `obj`=:O', {
            ':T': token,
            ':A': app,
            ':G': grp,
            ':O': obj,
            ':ATTR': attribute
        }).ok;
    }
}

function login(caller, token) {
    // caller.setPrivateData(JS, 'token', undefined);
    if (DB.exec('SELECT * FROM `account` WHERE `token`=:T', {
              ':T': token
          }).rows.length > 0) {
        var record = { ID: caller.__ID__, token: token }
        var found = loginedCallers.find((item) => item.ID == caller.__ID__);

        if (found !== undefined)
            found = record;
        else
            loginedCallers.push(record);

        // caller.setPrivateData(JS, 'token', token);
        return true;
    } else
        return false;
}

function logout(caller) {
    var found = loginedCallers.findIndex((item) => item.ID == caller.__ID__);
    if (found !== undefined)
        loginedCallers.splice(found, 1);
}

function getToken(caller) {
    var found = loginedCallers.find((item) => item.ID == caller.__ID__);
    if (found === undefined)
        return '';

    var token = found.token;

    console.log("----- getToken -------", caller.__ISINTERNALCALL__, caller.__GRP__, caller.__APP__, caller.__OBJECT__, caller.__OBJECTID__, token);

    // var token = caller.privateData(JS, 'token');
    var result = DB.exec('SELECT `token` FROM `access` WHERE `token`=:T AND `app`=:A AND `grp`=IFNULL(:G,"") AND (`obj`="*" OR `obj`=:O)', {
        ':T': token,
        ':A': caller.__APP__,
        ':G': caller.__GRP__,
        ':O': caller.__OBJECT__
    });

    if (result.rows.length > 0)
        return result.rows[0].token;
    else
        return '';
    //return caller.privateData(JS, 'token');
}

function accountList(caller) {
    var result = DB.exec('SELECT json_group_object(`token`, `nick`) AS `JSON` FROM account');

    if (result.rows.length > 0) {
        return result.rows[0].JSON;
    } else
        return "{}";
}

