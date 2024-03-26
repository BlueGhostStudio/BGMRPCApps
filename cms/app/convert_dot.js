methods = ["convert"];

function convert(caller, node) {
    var data = JS.call("graphviz", "render", [node.content])[0];
    if (data.ok)
        node.content = data.svg;
    return node;
}
