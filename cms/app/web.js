var PREFIX;
methods = [
    /*'traverse',
    'traverseCheck'*/
    'backtrackNode'
];

function constructor() {
    PREFIX = JS.__APP__ + '::';
    if (JS.__GRP__.length > 0)
        PREFIX = JS.__GRP__ + '::' + PREFIX;
}

/*function traverse(caller, node, targetPath, stopPath) {
    var stpNode;
    if (stopPath != undefined) {
        stpNode = JS.call(caller, PREFIX + 'main', 'nodeInfo', [stopPath])[0];
        if (!stpNode.ok)
            stpNode = undefined;
    }

    var curNode = JS.call(caller, PREFIX + 'main', 'nodeInfo', [node])[0];
    if (curNode.ok) {
        targetPath = targetPath.replace(/(^\s*\/\s*|\s*\/\s*$)/g, '');
        var tsp = targetPath.split('/');

        while(1) {
            var tNode = curNode;
            for (var x in tsp) {
                tNode = JS.call(caller, PREFIX + 'main', 'nodeInfo',
                    [tNode.id, tsp[x]])[0];
                if (!tNode.ok)
                    break;
            }

            if (tNode.ok)
                return { ok: true, node: curNode.node, target: tNode.node };

            if (!curNode.id || (stpNode != undefined && stpNode.id == curNode.id))
                return { ok: false, error: "No exist node" };

            curNode = JS.call(caller, 
                PREFIX + 'main', 'nodeInfo',
                [curNode.node.pid])[0];
        };
    } else
        return curNode;
}

function traverseCheck(caller, node, target, stopPath) {
    var stpNode;
    if (stopPath != undefined) {
        stpNode = JS.call(caller, PREFIX + 'main', 'nodeInfo', [stopPath])[0];
        if (!stpNode.ok)
            stpNode = undefined;
    }
    var curNode = JS.call(caller, PREFIX + 'main', 'nodeInfo', [node])[0];

    if (curNode.ok) {
        while(1) {
            if (curNode.node.name == target)
                return { ok: true, node: curNode.node };

            if (!curNode.node.pid || (stpNode != undefined && stpNode.id == curNode.id))
                return { ok: false, error: "No exist node" };

            curNode = JS.call(caller, PREFIX + 'main',
                'nodeInfo',
                [curNode.node.pid])[0];
        }
    } else
        return curNode;
}*/

function backtrackNode(caller, node, targetPath, stopPath) {
    var stpNode;
    if (stopPath != undefined) {
        stpNode = JS.call(caller, PREFIX + 'main', 'nodeInfo', [stopPath])[0];
        if (!stpNode.ok)
            stpNode = undefined;
    }

    var curNode = JS.call(caller, PREFIX + 'main', 'nodeInfo', [node])[0];
    if (curNode.ok) {
        targetPath = targetPath.replace(/(^\s*\/\s*|\s*\/\s*$)/g, '');

        /*var tsp = [targetPath];
        if (/(?<!\\)\//.test(targetPath))
            tsp = targetPath.split(/(?<!\\)\//);*/

        while(1) {
            /*var tNode = curNode;
            for (var x in tsp) {
                tNode = JS.call(caller, PREFIX + 'main', 'nodeInfo',
                    [tNode.id, tsp[x]])[0];
                if (!tNode.ok)
                    break;
            }*/
            var tNode = JS.call(caller, PREFIX + 'main', 'refNodeInfo',
                [curNode.id, targetPath])[0];

            if (tNode.ok)
                return { ok: true, node: curNode.node, target: tNode.node };

            if (!curNode.id || (stpNode != undefined && stpNode.id == curNode.id))
                return { ok: false, errno: 0, error: "No exist node" };

            curNode = JS.call(caller,
                PREFIX + 'main', 'nodeInfo', [curNode.node.pid])[0];
        }
    } else {
        curNode.errno = 1;
        return curNode;
    }
}

