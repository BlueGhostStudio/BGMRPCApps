var infoFields = '`pid`,`id`,`type`,`title`,`name`,`contentType`,`summary`,`extData`,json_extract(:AL, "$."||`own`) AS `own`,`hide`,`private`,`date`,`mdate`,`seq` ';
var rootNode = {
    pid: null,
    id: null,
    title: '/',
    type: 'D'
}

function getCallerToken(caller) {
    return JS.call(caller, 'account', 'getToken', [])[0];
}

function _nodeByID(id, token) {
    //console.log("id", id, "token", token);
    var result = DB.exec('SELECT * '
        + 'FROM VDIR '
        + 'WHERE `id`=:I AND (`own`=:T OR `private`=0)', {':I': id, ':T': token});

    if (result.ok && result.rows.length > 0) {
        var id = result.rows[0].id;
        if (id != null && id < 0) {
            id = -id;
            result.rows[0].id = id;
        }

        return {
            ok: true,
            id: id,
            node: result.rows[0]
        };
    } else
        return { ok: false, error: 'no exist node' };
}

function _nodeByName(pid, name, token, ref) {
    if (ref === undefined)
        ref = true;

    if (pid === '')
        pid = null;

    var result = DB.exec('SELECT * '
        + 'FROM VDIR '
        + 'WHERE IIF(:P IS NULL, `pid` IS NULL, `pid`=ABS(:P)) '
        + ' AND `name`=:N AND (`own`=:T OR `private`=0)',
        {
            ':P': pid,
            ':N': name,
            ':T': token
        });


    if (result.ok && result.rows.length > 0) {
        var row = result.rows[0];
        var id = row.id;
        if (id != null && id < 0) {
            id = -id;
            row.id = id;
        } else if (ref && row.type === 'R' && row.contentType === 'ref')
            return _refNode(token, id);

        return {
            ok: true,
            id: id,
            node: row
        };
    } else
        return { ok: false, error: 'no exist node' };
}

function _nodeByPath(path, token, pNode) {
    path = path.trim();

    var sp = path.split('/');
    var n = {ok: true, id: null, node: rootNode};
    if (pNode !== undefined && sp[0] !== '')
        n = pNode;

    for (var x in sp) {
        let name = sp[x];
        if (name !== '') {
            n = _nodeByName(n.id, name, token);

            if (!n.ok)
                break;
        }
    }

    return n;
}

function _node(token) {
    if (arguments.length == 2) {
        var n = arguments[1];
        if (n == null)
            return { ok: true, id: null, node: rootNode };
        else if (typeof n === 'number')
            return _nodeByID(n, token);
        else if (typeof n === 'string')
            return _nodeByPath(n, token);
        else
            return { ok: false, id: null, error: 'type error' };
    } else if (arguments.length > 2) {
        var pnode = _node(token, arguments[1]);
        if (!pnode.ok)
            return pnode;
        else {
            var name = arguments[2];
            if (typeof name === 'number')
                return _nodeByID(name, token);
            else if (name.search('/') >= 0)
                return _nodeByPath(name, token, pnode);
            else
                return _nodeByName(pnode.id, name, token, false);
        }
    } else
        return { ok: false, id: null, error: 'arguments count fail' };
}

function _refNode(token) {
    var n = _node.apply(null, arguments);

    while (n.ok && n.node.type === 'R' && n.node.contentType == 'ref') {
        var content = n.node.content.replace(/^ref:/, '');
        if (!isNaN(content))
            content = Number(content);

        n = _node(token, content);
    }

    return n;
}

function _search(query, token, accountList) {
    var cond_stm = '';
    var limit_stm = '';
    var bindValues = {
        ':T': token,
        ':AL': accountList
    }

    if (query != undefined) {
        if ('limit' in query && 'count' in query['limit']) {
            var limit = query['limit'];
            limit_stm += 'LIMIT ' + limit.count;
            if ('offset' in limit)
                limit_stm += ' OFFSET ' + limit.offset;
        }
        if ('type' in query) {
            if (query['type'] == 'D')
                cond_stm += ' AND `type`="D"';
            else if (query['type'] == 'F')
                cond_stm += ' AND (`type`="F" OR `type`="R")';
        }
        if ('cond' in query) {
            cond_stm += ' AND ' + query['cond'];
        }
        if ('nohide' in query)
            cond_stm += query['nohide'] ? ' AND `hide`=0' : '';
        if ('noDotDot' in query)
            cond_stm += query['noDotDot'] ? ' AND `id`>0' : '';
        if ('pid' in query) {
            cond_stm += ' AND IIF(:P IS NULL, `pid` IS NULL, `pid`=ABS(:P))';
            bindValues[":P"] = query['pid'];
        }
    }

    var result = DB.exec('SELECT ' + infoFields
        + 'FROM VDIR '
        + 'WHERE (`own`=:T OR `private`=0) ' + cond_stm + ' '
        + "ORDER BY IIF(`name`='..', 0, 1), IIF(`type`='D', 0, 1), IIF(`seq`=-1, 1, 0), `seq`, `date` DESC "
        + limit_stm,
        bindValues
    );

    return {
        ok: true, list: result.rows
    }
}

function _list(pNode, nohide, noDotDot, query, token, accountList) {
    if (query == undefined)
        query = {};

    pNode = _node(token, pNode);
    if (!pNode.ok)
        return pNode;
    else if (pNode.node.type != 'D')
        return { ok: false, error: 'the node is not directory node' };

    query['pid'] = pNode.id;
    query['nohide'] = nohide;
    query['noDotDot'] = noDotDot;

    var result = _search(query, token, accountList);
    result['path'] = _nodePath(pNode.id, token);
    result['id'] = pNode.id;
    return result;
    /*var limit_stm = '';
    var cond_stm = '';
    if (query != undefined) {
        if ('limit' in query && 'count' in query['limit']) {
            var limit = query['limit'];
            limit_stm = 'LIMIT ' + limit.count;
            if ('offset' in limit)
                limit_stm += ' OFFSET ' + limit.offset;
        }
        if ('type' in query) {
            if (query['type'] == 'D')
                cond_stm += ' AND `type`="D"';
            else if (query['type'] == 'F')
                cond_stm += ' AND (`type`="F" OR `type`="R")';
        }
    }

    pNode = _node(token, pNode);

    if (!pNode.ok)
        return pNode;
    else if (pNode.node.type != 'D')
        return { ok: false, error: 'the node is not directory node' };

    var result = DB.exec('SELECT ' + infoFields
        + 'FROM VDIR '
        + 'WHERE IIF(:P IS NULL, `pid` IS NULL, `pid`=ABS(:P)) '
        + 'AND (`own`=:T OR `private`=0) ' + (nohide ? 'AND `hide`=0 ' : ' ')
        + (noDotDot ? 'AND `id`>0 ' : ' ') + cond_stm
        + "ORDER BY IIF(`type`='D', 0, 1), IIF(`seq`=-1, 1, 0), `seq` " + limit_stm, {
            ':P': pNode.id,
            ':T': token,
            ':AL': accountList
        });

    return {
        ok: true, id: pNode.id,
        path: _nodePath(pNode.id, token),
        list: result.rows
    };*/
}

function _updateNode(n, d, token) {
    if (!token)
        return { ok: false, error: 'permission denied' };

    var n = _node(token, n);
    if (!n.ok)
        return n;
    else if (n.node.own !== token && n.node.own !== "BGCMS")
        return { ok: false, error: 'permission denied' };

    var nID = n.id;
    var pID = n.node.pid == '' ? null : n.node.pid;

    var isMove = false;
    if ('pid' in d) {
        var pn = _node(token, d['pid'])

        if (!pn.ok)
            return pn;
        else if (pn.id != null && pn.node.own !== token && pn.node.own !== "BGCMS")
            return { ok: false, error: 'permission denied' };
        else
            d.pid = pn.id;

        if (pn.id != pID) {
            var chpn = pn;
            while(chpn.id != null) {
                if (chpn.id == nID)
                    return { ok: false, error: 'cannot move node to a subdirectory of itself' };

                chpn = _node(token, chpn.node.pid);
            }

            isMove = true;
        } else {
            console.log("----same pnode----");
            delete d.pid;
        }
        /*if (pn.id != pID) {
            var chpn = pn;

            while(chpn.node.pid != null) {
                if (chpn.node.pid === nID)
                    return { ok: false, error: 'cannot move node to a subdirectory of itself' };

                chpn = _node(token, chpn.pid);
            }

            if (!pn.ok)
                return pn;
            else if (pn.id !== null && pn.node.own !== token)
                return { ok: false, error: 'permission denied' };
            else
                d.pid = pn.id;
            isMove = true;
        } else {
            console.log("------same pnode---");
            delete d.pid;
        }*/
    }

    var stf = updateStatementFragments([
        'pid',
        'name',
        'title',
        'summary',
        'extData',
        'content',
        'contentType',
        'hide',
        'private',
        'seq'
    ], d);
    stf.bindValues[':I'] = nID;

    if (Object.keys(stf.bindValues).length == 1)
        return { ok: false, error: 'no data update' }

    var result = DB.exec('UPDATE `VDIR` SET ' + stf.stm + ' WHERE `id`=:I', stf.bindValues);
    if (result.ok) {
        /*if ('pid' in d && d.pid != pID)
            JS.emitSignal('nodeMoved', [nID, d.pid])
        if ('name' in d)
            JS.emitSignal('nameUpdated', [nID, d.name]);
        if ('title' in d)
            JS.emitSignal('titleUpdated', [nID, d.title]);
        if ('content' in d)
            JS.emitSignal('contentUpdated', [nID, d.content]);
        if ('contentType' in d)
            JS.emitSignal('contentTypeUpdated', [nID, d.contentType]);
        if ('hide' in d)
            JS.emitSignal('hideUpdated', [nID, d.hide]);
        if ('private' in d)
            JS.emitSignal('privateUpdated', [nID, d.private]);
        if ('seq' in d)
            JS.emitSignal('seqUpdated', [nID, d.seq]);*/

        var updatedNode = _node(token, nID);
        if ('content' in d)
            JS.emitSignal('contentUpdated', [nID, d.content]);
        if ('summary' in d)
            JS.emitSignal('summaryUpdated', [nID, d.summary]);
        if ('extData' in d)
            JS.emitSignal('extDataUpdated', [nID, d.extData]);
        if ('title' in d)
            JS.emitSignal('titleUpdated', [nID, d.title]);

        delete updatedNode.node["content"];
        updatedNode.move = isMove;

        if ('pid' in d && d.pid != pID)
            JS.emitSignal('nodeMoved', [updatedNode, d.pid]);

        /*if ('title' in d || 'contentType' in d || 'hide' in d
            || 'private' in d || 'seq' in d)
            JS.emitSignal("nodeUpdated", [nID, updatedNode]);*/
        var ks = Object.keys(d);
        if ('name' in d)
            JS.emitSignal('nodeRenamed', [nID, d['name']]);
        if (ks.some(elem=>['name', 'title', 'summary', 'extData', 'contentType', 'hide',
            'private', 'seq'].includes(elem)))
            JS.emitSignal('nodeUpdated', [nID, updatedNode]);

        return updatedNode;
    } else
        return result;
}

function _copyNode(s, t, token, ref) {
    if (!token)
        return { ok: false, error: 'permission denied' };

    s = _refNode(token, s);
    if (!s.ok)
        return s;
    else if (s.id === null)
        return { ok: false, error: 'cannot copy root node' };
    else if (s.node.own !== token && s.node.own !== "BGCMS")
        return { ok: false, error: 'permission denied' };
    var sID = s.id;
    var sPID = s.node.pid === '' ? null : s.node.pid;
    var sName = s.node.name;

    t = _node(token, t);
    if (!t.ok)
        return t;
    else if (t.id !== null && t.node.own !== token && t.node.own !== "BGCMS")
        return { ok: false, error: 'permission denied' };
    var tID = t.id;

    if (sPID !== tID) {
        var cht = t;
        while(cht.id !== null) {
            if (cht.id === sID)
                return { ok: false, error: 'cannot copy node to a subdirectory of itself' };

            cht = _node(token, cht.node.pid);
        }
    } else
        sName += '_copy_' + randomStr(6);

    var result = DB.exec('INSERT INTO `VDIR` ' + 
        '(`pid`, `name`, `type`, `title`, ' +
        '`content`, `contentType`, ' +
        '`own`, `private`, `hide`, `extData`) ' +
        'SELECT :P AS `pid`, :N AS `name`, ' +
        (ref ? '"R" AS `type`, ' : '`type`, ') +
        '`title`, ' +
        (ref ? '"ref:"||`id` AS `content`, ' : '`content`, ') +
        (ref ? '"ref" AS `contentType`, ' : '`contentType`, ') +
        '`own`, `private`, `hide`, ' +
        (ref ? '`type` AS `extData` ' : '`extData` ') +
        'FROM `VDIR` ' +
        'WHERE `id`=:I', {
            ':P': tID,
            ':N': sName,
            ':I': sID
        });

    if (result.ok) {
        var newNode = _node(token, tID, sName);
        JS.emitSignal('nodeCopied', [newNode]);

        if (s.node.type === 'D' && newNode.node.type === 'D') {
            var subList = _list(sID, false, true, undefined, token);

            if (subList.ok) {
                subList = subList.list;
                for (let x in subList) {
                    _copyNode(subList[x].id, newNode.id, token);
                }
            }
        }

        return newNode;
    } else
        return result;
}

function _nodePath(node, token) {
    var path = {
        str: "",
        ids: [],
        titles: []
    }

    node = _node(token, node);
    while (node.ok && node.id != null) {
        path.str = node.node.name + (path.str == '' ? '' : '/' + path.str);
        path.ids.unshift(node.id);
        path.titles.unshift(node.node.title || node.node.name);
        node = _node(token, node.node.pid);
    }

    path.ids.unshift(null);
    path.titles.unshift('/');
    path.str = '/' + path.str;

    return path;
}
/*function _nodePath(node, token) {
    var path = "";

    node = _node(token, node);
    while (node.ok && node.id != null) {
        path = node.node.name + (path == '' ? '' : '/' + path);
        node = _node(token, node.node.pid);
    }

    return '/' + path;
}

function _nodePathIDs(node, token) {
    var path = [];

    node = _node(token, node);
    while (node.ok && node.id != null) {
        path.unshift(node.id);
        node = _node(token, node.node.pid);
    }

    return path;
}*/

