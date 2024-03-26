// private:
var buffer = [];
var FILE;
//var MediaPath;

methods = [
    'requestPostMedia', 'writeMediaData', 'writeMediaDataEnd', 'requestGetMedia',
    'readMediaData', 'removeMedia', 'close', 'mediaURL'
];

function constructor() {
    JS.include('../common/randomStr.js');
    JS.include('mediaConfig.js');
    JS.loadModule('jsFile');
    JS.loadModule('jsByteArray');

    FILE = new JsFile;
    if (!FILE.exists(MediaPath)) {
        FILE.mkpath(MediaPath);
        FILE.copy(JS.__PATH_APP__ + '/logo.png', MediaPath + '/logo.png');
    }
    /*MediaPath = JS.__PATH_DATA__ + '/' +
        'media/';*/
}

function requestPostMedia(caller, type, imgFile) {
    var id;
    if (imgFile === undefined)
        id = randomStr(6) + '.' + type;
    else {
        id = imgFile;
        if (id in buffer)
            close(id)
    }

    buffer[id] = new JsByteArray(true);

    return id;
}

function writeMediaData(caller, id, base64) {
    var ba = new JsByteArray;
    ba.fromBase64(base64);
    buffer[id].append(ba);
    return true;
}

function writeMediaDataEnd(caller, id) {
    FILE.writeFile(MediaPath + id, buffer[id].data());
    close(caller, id);
}

function requestGetMedia(caller, media) {
    var id = randomStr(6);
    var data = JsByteArray(FILE.readFile(MediaPath + media), true);

    if (data.size() > 0) {
        buffer[id] = data;
        return [id, data.size()];
    } else {
        JS.destroyObject(data);
        return false;
    }
}

function readMediaData(caller, id, pos, len) {
    var data = buffer[id].mid(pos, len);
    pos += data.size();
    if (pos >= buffer[id].size()) {
        pos = -1;
        close(caller, id);
    }
    return {data: data.toBase64().data(), pos: pos};
}

function removeMedia(caller, media) {
    return FILE.removeFile(MediaPath + media);
}

function close(caller, id) {
    if (id in buffer) {
        JS.destroyObject(buffer[id]);
        delete buffer[id];
        return true;
    } else
        return false;
}

function mediaURL(caller, media) {
    /*var subDir = 'default';
    if (JS.__GRP__.length > 0)
        subDir = 'imgs/' + JS.__GRP__
    return mediaUrl + '/' + subDir + '/' + encodeURIComponent(media);*/
    return mediaUrl + '/' + encodeURIComponent(media);
}

