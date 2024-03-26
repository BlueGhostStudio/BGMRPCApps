#include "bgcms.h"

#include <bgmrpccommon.h>
#include <mthdAdaptIF.h>

#include <QSqlError>

#include "caller.h"

using namespace NS_BGMRPCObjectInterface;

t_retBase::t_retBase(bool ok) : m_ok(ok) {}

t_retBase::t_retBase(const QString& error) : m_ok(false), m_error(error) {}

bool
t_retBase::ok() const {
    return m_ok;
}

QString
t_retBase::error() const {
    return m_error;
}

t_retBase::operator QVariant() const { return toMap(); }

const QVariantMap&
t_retBase::attr() const {
    return m_attributes;
}

QVariantMap&
t_retBase::attr() {
    return m_attributes;
}

void
t_retBase::insertAttrs(QVariantMap& map) const {
    map.insert(m_attributes);
}

/* ------------------------------------------- */

t_retNode::t_retNode() : t_retBase() {}

t_retNode::t_retNode(const QVariantMap& node) : t_retBase(true), m_node(node) {}

t_retNode::t_retNode(const QString& error) : t_retBase(error) {}

QVariant
t_retNode::ID() const {
    return m_node["id"];
}

int
t_retNode::nID() const {
    return m_node["id"].toInt();
}

void
t_retNode::setID(int id) {
    m_node["id"] = id;
}

QVariant
t_retNode::pID() const {
    return m_node["pid"];
}

int
t_retNode::nPID() const {
    return m_node["pid"].toInt();
}

const QVariantMap
t_retNode::toMap() const {
    QVariantMap ret;
    if (m_ok) {
        ret = QVariantMap{ { "ok", true },
                           { "id", m_node["id"] },
                           { "node", m_node } };
        insertAttrs(ret);
    } else
        ret = QVariantMap{ { "ok", false }, { "error", m_error } };

    return ret;
}

const QVariantMap&
t_retNode::node() const {
    return m_node;
}

QVariantMap&
t_retNode::node() {
    return m_node;
}

QVariant
t_retNode::operator[](const QString& key) const {
    return m_node[key];
}

QVariant&
t_retNode::operator[](const QString& key) {
    return m_node[key];
}

/* -------------------------------------- */

t_retList::t_retList() : t_retBase() {}

t_retList::t_retList(const QVariantList& list)
    : t_retBase(true), m_list(list) {}

t_retList::t_retList(const QString& error) : t_retBase(error) {}

const QVariantMap
t_retList::toMap() const {
    QVariantMap ret;
    if (m_ok) {
        ret = QVariantMap{ { "ok", true }, { "list", m_list } };
        insertAttrs(ret);
    } else
        ret = QVariantMap{ { "ok", false }, { "error", m_error } };

    return ret;
}

const QVariantList&
t_retList::list() const {
    return m_list;
}

QVariantList&
t_retList::list() {
    return m_list;
}

/* -------------------------------------- */

BGCMS::BGCMS(QObject* parent) : ObjectInterface(parent) {
    QMetaType::registerConverter<t_retNode, QVariantMap>(
        [](const t_retNode& node) { return node.toMap(); });
    QMetaType::registerConverter<t_retList, QVariantMap>(
        [](const t_retList& list) { return list.toMap(); });
}

QString
BGCMS::getCallerToken(QPointer<Caller> caller) {
    return call(caller, "account", "getToken", QVariantList())
        .toList()[0]
        .toString();
}

QVariant
BGCMS::node(QPointer<Caller> caller, const QVariantList& args) {
    return node(getCallerToken(caller), args);
}

QVariant
BGCMS::refNode(QPointer<Caller> caller, const QVariantList& args) {
    QString token = getCallerToken(caller);

    return refNode(token, args);
}

QVariant
BGCMS::nodeInfo(QPointer<Caller> caller, const QVariantList& args) {
    t_retNode retNode = node(getCallerToken(caller), args);

    if (retNode.ok()) retNode.node().remove("content");

    return retNode;
}

QVariant
BGCMS::refNodeInfo(QPointer<Caller> caller, const QVariantList& args) {
    t_retNode retNode = refNode(getCallerToken(caller), args);

    if (retNode.ok()) retNode.node().remove("content");

    return retNode;
}

QVariant
BGCMS::exists(QPointer<Caller> caller, const QVariantList& args) {
    return node(getCallerToken(caller), args).ok();
}

QVariant
BGCMS::CNode(QPointer<Caller> caller, const QVariantList& args) {
    t_retNode theNode = node(getCallerToken(caller), args);

    if (!theNode.ok()) return theNode;

    QByteArray convertObj =
        genObjectName(m_grp.toLatin1(), m_appName.toLatin1(),
                      "convert_" + theNode["contentType"].toByteArray(), false);

    if (objCtrlCmd((quint8)NS_BGMRPC::CTRL_CHECKOBJECT, convertObj)[0]) {
        theNode.node() =
            call(convertObj, "convert", { theNode.node() }).toList()[0].toMap();
        return theNode;
    } else
        return theNode;
}

QVariant
BGCMS::CRefNode(QPointer<Caller> caller, const QVariantList& args) {
    t_retNode theNode = refNode(getCallerToken(caller), args);

    if (!theNode.ok()) return theNode;

    QByteArray convertObj =
        genObjectName(m_grp.toLatin1(), m_appName.toLatin1(),
                      "convert_" + theNode["contentType"].toByteArray(), false);

    if (objCtrlCmd((quint8)NS_BGMRPC::CTRL_CHECKOBJECT, convertObj)[0]) {
        theNode.node() =
            call(convertObj, "convert", { theNode.node() }).toList()[0].toMap();
        return theNode;
    } else
        return theNode;
}

QVariant
BGCMS::search(QPointer<Caller> caller, const QVariantMap& query, int filter) {
    return search(caller, getCallerToken(caller), query, filter);
}

QVariant
BGCMS::list(QPointer<Caller> caller, const QVariant& pNode, int filter,
            QVariantMap query) {
    QString token = getCallerToken(caller);

    t_retNode retPNode = node(token, { pNode });

    if (!retPNode.ok())
        return t_retList("no exists parent node");
    else if (retPNode["type"] != "D")
        return t_retList("the node is not directory node");

    query["pid"] = retPNode.ID();

    t_retList listRet = search(caller, token, query, filter);
    listRet.attr()["id"] = retPNode.ID();
    listRet.attr()["path"] = nodePath(caller, retPNode.ID());

    return listRet;
}

QVariant
BGCMS::newNode(QPointer<Caller> caller, const QVariant& dir,
               const QVariantMap& data) {
    QString token = getCallerToken(caller);
    if (token.isEmpty()) return t_retNode("permission denied");

    t_retNode dirNode = node(token, { dir });
    if (!dirNode.ok())
        return dirNode;
    else if (dirNode["type"] == "F")
        return t_retNode{ "target node is not a directory" };
    else if (dirNode.ID().isValid() && dirNode["own"] != "BGCMS" &&
             dirNode["own"] != token)
        return t_retNode{ "permission denied" };

    QVariantMap stf = insertStatementFragments({ { "pid", dirNode["id"] },
                                                 { "type", "F" },
                                                 { "title", QVariant() },
                                                 { "summary", QVariant() },
                                                 { "extData", QVariant() },
                                                 { "name", QVariant() },
                                                 { "content", QVariant() },
                                                 { "contentType", QVariant() },
                                                 { "own", token },
                                                 { "hide", 0 },
                                                 { "private", 0 },
                                                 { "seq", -1 } },
                                               data, nullptr);

    QSqlQuery query(m_db);
    query.prepare(R"sql(
INSERT INTO `VDIR` ()sql" +
                  stf["fstm"].toString() + ") VALUES (" +
                  stf["vstm"].toString() + ")");

    QVariantMap bindValues = stf["bindValues"].toMap();
    QVariantMap::iterator it;
    for (it = bindValues.begin(); it != bindValues.end(); ++it)
        query.bindValue(it.key(), it.value());

    bool ok;
    {
        QMutexLocker locker(&m_BGCMSMutex);
        ok = query.exec();
    }
    if (ok) {
        t_retNode newNode = node(token, dirNode.ID(), data["name"].toString());
        emitSignal("nodeCreated", QVariantList{ newNode });

        return newNode;
    } else
        return t_retNode("sql error");
}

QVariant
BGCMS::updateNode(QPointer<Caller> caller, const QVariant& target,
                  const QVariantMap& data) {
    QString token = getCallerToken(caller);
    if (token.isEmpty()) return t_retNode("permission denied");

    t_retNode targetNode = node(token, { target });

    if (!targetNode.ok())
        return targetNode;
    else if (targetNode["own"] != token && targetNode["own"] != "BGCMS")
        return t_retNode("permission denied");

    QVariant targetNID = targetNode.ID();
    QVariant targetPID = targetNode.pID();
    if (targetPID.toString().isEmpty()) targetPID = QVariant();

    bool isMove = false;
    if (data.contains("pid") && data["pid"] != targetPID) {
        t_retNode dataPNode = node(token, { data["pid"] });
        if (!dataPNode.ok())
            return dataPNode;
        else if (dataPNode.ID().isValid()) {
            QString own = dataPNode["own"].toString();
            if (own != token && own != "BGCMS")
                return t_retNode("permission denied");
        }

        while (dataPNode.ID().isValid()) {
            if (dataPNode.ID() == targetNID)
                return t_retNode(
                    "cannot move node to a subdirectory of itself");

            dataPNode = node(token, { dataPNode.pID() });
        }

        isMove = true;
    }

    QVariantMap stf = updateStatementFragments(
        { "pid", "name", "title", "summary", "extData", "content",
          "contentType", "hide", "private", "seq" },
        data, nullptr);
    QString stm = stf["stm"].toString();
    QVariantMap bindValues = stf["bindValues"].toMap();
    if (bindValues.isEmpty()) return t_retNode("no data update");

    bindValues[":I"] = targetNID;

    // QMutexLocker locker(&m_BGCMSMutex);
    QSqlQuery query(m_db);

    query.prepare(R"sql(
UPDATE `VDIR` set )sql" +
                  stf["stm"].toString() + R"sql(
WHERE `id`=:I)sql");

    QVariantMap::iterator it;
    for (it = bindValues.begin(); it != bindValues.end(); ++it)
        query.bindValue(it.key(), it.value());

    bool ok;
    {
        QMutexLocker locker(&m_BGCMSMutex);
        ok = query.exec();
    }

    if (ok) {
        t_retNode updatedNode = node(token, targetNID.toInt());
        if (data.contains("content"))
            emitSignal("contentUpdated",
                       QVariantList{ targetNID, data["content"] });
        if (data.contains("summary"))
            emitSignal("summaryUpdated",
                       QVariantList{ targetNID, data["summary"] });
        if (data.contains("extData"))
            emitSignal("extDataUpdated",
                       QVariantList{ targetNID, data["extDataUpdated"] });
        if (data.contains("title"))
            emitSignal("titleUpdated",
                       QVariantList{ targetNID, data["title"] });

        updatedNode.node().remove("content");
        updatedNode.attr()["move"] = isMove;

        if (isMove)
            emitSignal("nodeMoved", QVariantList{ updatedNode, data["pid"] });
        emitSignal("nodeUpdated", QVariantList{ targetNID, updatedNode });

        return updatedNode;
    } else
        return t_retNode("sql error");
}

QVariant
BGCMS::removeNode(QPointer<Caller> caller, const QVariant& target) {
    QString token = getCallerToken(caller);
    if (token.isEmpty()) return t_retNode("permission denied");

    t_retNode targetNode = node(token, { target });
    if (targetNode.ok()) {
        if (targetNode["own"] != token && targetNode["own"] != "BGCMS")
            return t_retNode("permission denied");

        QSqlQuery query(m_db);
        query.prepare(R"sql(
DELETE FROM `VDIR` WHERE `id`=:I)sql");
        query.bindValue(":I", targetNode.nID());

        bool ok;
        {
            QMutexLocker locker(&m_BGCMSMutex);
            ok = query.exec();
        }
        if (ok) {
            emitSignal("nodeRemoved", { targetNode.nID() });
            if (targetNode["type"] == "R")
                emit caller->emitSignal(
                    "resourceNodeRemoved",
                    QVariantList{ targetNode.ID(), targetNode["contentType"],
                                  targetNode["content"] });

            return QVariantMap{ { "ok", true } };
        } else
            return t_retNode("sql error");
    } else
        return targetNode;
}

QVariant
BGCMS::copyNode(QPointer<Caller> caller, const QVariant& source,
                const QVariant& target, bool ref) {
    QString token = getCallerToken(caller);
    if (token.isEmpty()) return t_retNode("permission denied");

    t_retNode srcNode = refNode(token, { source });
    if (!srcNode.ok())
        return srcNode;
    else if (srcNode.ID().isNull())
        return t_retNode("cannot copy root node");
    else if (srcNode["own"] != token && srcNode["own"] != "BGCMS")
        return t_retNode("permission denied");

    t_retNode targetNode = node(token, { target });
    if (!targetNode.ok())
        return targetNode;
    else if (targetNode.ID().isValid() && targetNode["own"] != token &&
             targetNode["own"] != "BGCMS")
        return t_retNode("permission denied");
    else if (targetNode["type"] != "D")
        return t_retNode("target node is not a directory");

    QVariant srcNID = srcNode.ID();
    QVariant srcPID = srcNode.pID();
    QString srcName = srcNode["name"].toString();
    QVariant targetID = targetNode.ID();

    if (srcPID != targetID) {
        if (srcNode["type"] == "D") {
            t_retNode checkTarget = targetNode; /* Test if the target directory
                                                 * is the source directory
                                                 * or its subdirectories */
            while (checkTarget.ID().isValid()) {
                if (checkTarget.ID() == srcNID)
                    return t_retNode(
                        "The source node is a directory and cannot be "
                        "copied "
                        "into itself or any of its subdirectories.");

                checkTarget = node(token, { checkTarget.pID() });
            }
        }
    } else
        srcName =
            "_copy_" + QString::number(QRandomGenerator::global()->generate());

    QSqlQuery query(m_db);
    query.prepare(QString(R"sql(
INSERT INTO `VDIR`
    (`pid`, `name`, `type`, `title`,
     `content`, `contentType`, `own`,
     `private`, `hide`, `extData`)
SELECT :P AS `pid`, :N AS `name`, %1, `title`,
       %2, %3, `own`, `private`, `hide`,
       %4
FROM `VDIR`
WHERE `id`=:I)sql")
                      .arg(ref ? "\"R\" AS `type`" : "`type`")
                      .arg(ref ? "\"ref:\"||`id` AS `content`" : "`content`")
                      .arg(ref ? "\"ref\" AS `contentType`" : "`contentType`")
                      .arg(ref ? "`type` AS `extData`" : "`extData`"));
    query.bindValue(":P", targetID);
    query.bindValue(":N", srcName);
    query.bindValue(":I", srcNID);

    bool ok;
    {
        QMutexLocker locker(&m_BGCMSMutex);
        ok = query.exec();
    }
    if (ok) {
        t_retNode newNode = node(token, targetID, srcName, false);
        emitSignal("nodeCopied", QVariantList{ newNode });

        if (srcNode["type"] == "D") {
            t_retList srcSubList =
                list(caller, srcNID, 1, QVariantMap()).value<t_retList>();

            if (srcSubList.ok()) {
                foreach (const QVariant& subNode, srcSubList.list())
                    copyNode(caller, subNode.toMap()["id"], newNode.ID(),
                             false);
            }
        }

        return newNode;
    } else
        return t_retNode("sql error - " + query.lastError().text());
}

QVariant
BGCMS::copyRefNode(QPointer<Caller> caller, const QVariant& source,
                   const QVariant& target) {
    return copyNode(caller, source, target, true);
}

QVariant
BGCMS::moveNode(QPointer<Caller> caller, const QVariant& source, int target) {
    return updateNode(caller, source, QVariantMap{ { "pid", target } });
}

QVariant
BGCMS::nodePath(QPointer<Caller> caller, const QVariant& node) {
    QString token = getCallerToken(caller);

    QString str;
    QVariantList ids;
    QStringList titles;
    t_retNode iterNode = this->node(token, { node });
    while (iterNode.ok() && iterNode.ID().isValid()) {
        str = iterNode["name"].toString() + (str.isEmpty() ? "" : '/' + str);
        ids.prepend(iterNode.ID());
        QString title = iterNode["title"].toString();
        titles.prepend(!title.isEmpty() ? title : iterNode["name"].toString());

        iterNode = this->node(token, { iterNode.pID() });
    }

    ids.prepend(QVariant());
    titles.prepend("/");
    str = "/" + str;

    return QVariantMap{ { "str", str }, { "ids", ids }, { "titles", titles } };
}

t_retNode
BGCMS::node(const QString& token, const QVariantList& args) {
    // return t_retNode{ QVariantMap{ { "token", token }, { "args", args } }
    // };
    if (args.length() == 1) {
        QVariant vNode = args[0];
        if (vNode.isNull())
            return t_retNode(m_rootNode);
        else if (vNode.typeId() == QMetaType::QString)
            return node(token, vNode.toString());
        else if (isNumber(vNode))
            return node(token, vNode.toInt());
        else
            return t_retNode("type error");
    } else if (args.length() > 1) {
        t_retNode pNode = node(token, QVariantList{ args[0] });
        if (!pNode.ok())
            return t_retNode("parent node not exists");
        else {
            QVariant vNode = args[1];
            QString name_path = vNode.toString();

            if (isNumber(vNode))
                return node(token, vNode.toInt());
            else if (name_path.contains('/'))
                return node(token, name_path, pNode);
            else
                return node(token, pNode.ID(), name_path, false);
        }
    } else
        return t_retNode("arguments count fail");
    /*if (args.length() == 1) {
        QVariant vNode = args[0];
        if (vNode.isNull())
            return t_retNode(m_rootNode);
        else if (vNode.metaType().id() == QMetaType::QString)
            return node(token, vNode.toString());
        else if (isNumber(vNode))
            return node(token, vNode.toInt());
        else
            return t_retNode("type error");
    } else if (args.length() > 1) {
        t_retNode pNode = node(token, QVariantList{ args[0] });
        if (!pNode.ok())
            return pNode;
        else {
            QVariant vNode = args[1];
            QString name = vNode.toString();

            if (isNumber(vNode))
                return node(token, vNode.toInt());
            else if (name.contains('/'))
                return node(token, name, pNode);
            else
                return node(token, pNode.ID(), name, false);
        }
    } else
        return t_retNode("arguments count fail");*/
}

/* by m_id */
t_retNode
BGCMS::node(const QString& token, int id) {
    QSqlQuery query(m_db);

    query.prepare(R"sql(
SELECT *
FROM `VDIR`
WHERE `id`=:I)sql");
    query.bindValue(":I", id);

    bool ok;
    {
        QMutexLocker locker(&m_BGCMSMutex);
        ok = query.exec();
    }

    if (ok && query.first()) {
        t_retNode row = rowData(query.record());

        if (isNumber(row.ID()) && row.nID() < 0) row.setID(-row.nID());

        return row;
    } else
        return t_retNode("no exist node");
}

// by pID/name
t_retNode
BGCMS::node(const QString& token, QVariant pID, const QString& name, bool ref) {
    if (pID.metaType().id() == QMetaType::QString && pID.toString().isEmpty())
        pID = QVariant();

    QSqlQuery query(m_db);

    query.prepare(R"sql(
SELECT *
FROM VDIR
WHERE IIF(:P IS NULL, `pid` IS NULL, `pid`=ABS(:P))
    AND `name`=:N AND (`own`=:T OR `private`=0))sql");
    query.bindValue(":P", pID);
    query.bindValue(":N", name);
    query.bindValue(":T", token);

    bool ok;
    {
        QMutexLocker locker(&m_BGCMSMutex);
        ok = query.exec();
    }

    if (ok && query.first()) {
        t_retNode row = rowData(query.record());

        if (isNumber(row.ID()) && row.nID() < 0)
            row.setID(-row.nID());
        else if (ref && row["type"] == "R" && row["contentType"] == "ref")
            row = refNode(token, { row.ID() });

        return t_retNode(row);
    } else
        return t_retNode("no exist node");
}

/* by path */
t_retNode
BGCMS::node(const QString& token, const QString& path, const t_retNode& pNode) {
    t_retNode retNode(m_rootNode);
    QStringList sp = path.trimmed().split("/");

    if (pNode.ok() && !sp[0].isEmpty()) retNode = pNode;

    foreach (const QString& name, sp) {
        if (!name.isEmpty()) {
            retNode = node(token, retNode.ID(), name);

            if (!retNode.ok()) break;
        }
    }

    return retNode;
}

t_retList
BGCMS::search(QPointer<Caller> caller, const QString& token,
              const QVariantMap& query, int filter) {
    bool nohide = true;
    bool noDotDot = true;

    if (filter >= 0) {
        nohide = filter & 0x01;
        noDotDot = filter & 0x02;
    }

    QString accountListJson =
        call(caller, "account", "accountList", {}).toList()[0].toString();

    QString cond_stm = "";
    QString limit_stm = "";
    QVariantMap bindValues{ { ":T", token }, { ":AL", accountListJson } };

    if (!query.isEmpty()) {
        if (query.contains("limit") &&
            query["limit"].toMap().contains("count")) {
            QVariantMap vmLimit = query["limit"].toMap();
            limit_stm += "LIMIT " + vmLimit["count"].toString();
            if (vmLimit.contains("offset"))
                limit_stm += " OFFSET " + vmLimit["offset"].toString();
        }
        if (query.contains("type")) {
            QString sType = query["type"].toString();
            if (sType == "D")
                cond_stm += " AND `type`=\"D\"";
            else if (sType == "F")
                cond_stm += " AND (`type`=\"F\" OR `type`=\"R\")";
        }
        if (query.contains("cond"))
            cond_stm += " AND " + query["cond"].toString();
        cond_stm += nohide ? " AND `hide`=0" : "";
        cond_stm += noDotDot ? " AND `id`>0" : "";
        if (query.contains("pid")) {
            cond_stm += " AND IIF(:P IS NULL, `pid` IS NULL, `pid`=ABS(:P))";
            bindValues[":P"] = query["pid"];
        }
    }

    QSqlQuery sqlQuery(m_db);
    // clang-format off
    QString queryStm =
        "SELECT " + infoFields +
        " FROM `VDIR` " +
        "WHERE (`own`=:T OR `private`=0) " + cond_stm + " " +
        "ORDER BY IIF(`name`='D', 0, 1), " +
                 "IIF(`type`='D', 0, 1), " +
                 "IIF(`seq`=-1, 1, 0), `seq`, `date` DESC " +
        limit_stm;
    // clang-format on
    sqlQuery.prepare(queryStm);

    QVariantMap::iterator it;
    for (it = bindValues.begin(); it != bindValues.end(); ++it)
        sqlQuery.bindValue(it.key(), it.value());

    bool ok;
    {
        QMutexLocker locker(&m_BGCMSMutex);
        ok = sqlQuery.exec();
    }

    if (ok)
        return getRows(sqlQuery);
    else
        return t_retList{ "sql error" };
}

t_retNode
BGCMS::refNode(const QString& token, const QVariantList& args) {
    t_retNode retNode = node(token, args);
    while (retNode.ok() && retNode["type"] == "R" &&
           retNode["contentType"] == "ref") {
        QVariant vContent = retNode["content"].toString().replace(
            QRegularExpression("^ref:"), "");
        bool ok;
        int refID = vContent.toInt(&ok);

        retNode = node(token, { ok ? refID : vContent });
    }

    return retNode;
}

QVariantList
BGCMS::getRows(QSqlQuery& query) {
    QVariantList rows;

    if (query.isSelect()) {
        while (query.next()) {
            rows.append(rowData(query.record()).node());
        }
    }

    return rows;
}

t_retNode
BGCMS::rowData(const QSqlRecord& record) {
    QVariantMap row;
    for (int i = 0; i < record.count(); ++i) {
        row[record.fieldName(i)] = record.value(i);
    }
    return t_retNode(row);
}

QVariantMap
BGCMS::updateStatementFragments(
    const QStringList& fields, QVariantMap data,
    std::function<QVariant(const QString&,
                           const QStringList&,  // clang-format off
                           const QVariantMap&)> special) {  // clang-format on
    // bool first = true;
    QString sStm = "";
    QVariantMap bindValues;

    foreach (const QString& fieldName, fields) {
        if (data.contains(fieldName)) {
            if (!sStm.isEmpty()) sStm += ',';

            QString bindFieldName = ":" + fieldName;
            sStm += "`" + fieldName + "`=" + bindFieldName;

            QVariant spValue;
            if (special) spValue = special(fieldName, fields, data);

            if (spValue.isNull())
                bindValues[bindFieldName] = data[fieldName];
            else
                bindValues[bindFieldName] = spValue;
        }
    }

    return { { "stm", sStm }, { "bindValues", bindValues } };
}

QVariantMap
BGCMS::insertStatementFragments(
    const QVariantMap& initData, const QVariantMap& data,
    std::function<QVariant(const QString&,
                           const QVariantMap&,  // clang-format off
                           const QVariantMap&)> special) {  // clang-format on
    QVariantMap::const_iterator it;
    QString fStm;
    QString vStm;
    QVariantMap bindValues;

    for (it = initData.constBegin(); it != initData.constEnd(); ++it) {
        if (!fStm.isEmpty()) {
            fStm += ',';
            vStm += ',';
        }

        QString fieldName = it.key();
        QString bindFieldName = ':' + fieldName;
        fStm += '`' + fieldName + '`';
        vStm += bindFieldName;

        QVariant fieldValue = data[fieldName];
        QVariant spValue;
        if (special)
            fieldValue = special(fieldName, initData, data);
        else if (!fieldValue.isValid())
            fieldValue = initData[fieldName];

        if (data[fieldName].isValid())
            fieldValue = data[fieldName];
        else
            fieldValue = initData[fieldName];

        bindValues[bindFieldName] = fieldValue;
    }

    return { { "fstm", fStm }, { "vstm", vStm }, { "bindValues", bindValues } };
}

bool
BGCMS::isNumber(const QVariant& data) const {
    switch (data.typeId()) {
    case QMetaType::Short:
    case QMetaType::UShort:
    case QMetaType::Int:
    case QMetaType::UInt:
    case QMetaType::Long:
    case QMetaType::ULong:
    case QMetaType::LongLong:
    case QMetaType::ULongLong:
    case QMetaType::Float:
    case QMetaType::Double:
        return true;
    default:
        return false;
    }
}

bool
BGCMS::initial(int, char**) {
    m_db.setDatabaseName(m_dataPath + "/cms.db");
    qInfo() << "open database" << m_db.open();
    qDebug() << "----->" << m_dataPath + "/cms.db";

    return ObjectInterface::initial(0, nullptr);
}

using T_BGCMS_METHOD = QVariant (BGCMS::*)(QPointer<Caller>,
                                           const QVariantList&);
void
BGCMS::registerMethods() {
    // clang-format off
// =====================================================================================
    RMV("node", { "Get node" },
        &BGCMS::node);
// -------------------------------------------------------------------------------------
    RMV("refNode", { "Get reference node" },
        &BGCMS::refNode);
// -------------------------------------------------------------------------------------
    RMV("nodeInfo", { "Get node info" },
        &BGCMS::nodeInfo);
// -------------------------------------------------------------------------------------
    RMV("refNodeInfo", { "Get reference node info" },
        &BGCMS::refNodeInfo);
// -------------------------------------------------------------------------------------
    RMV("exists", { "Exists" },
        &BGCMS::exists);
// = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
    RMV("CNode", { "Get node and cover" },
        &BGCMS::CNode);
// -------------------------------------------------------------------------------------
    RMV("CRefNode", { "Get reference node and cover" },
        &BGCMS::CRefNode);
// =====================================================================================
     RM("search", { "Search node" },
        &BGCMS::search, ARG<QVariantMap>("query"), ARG<int>("filter", 0));
// -------------------------------------------------------------------------------------
     RM("list", { "List nodes of the base path." },
        &BGCMS::list, ARG("basePath"), ARG<int>("filter", 0), ARG<QVariantMap>("query", QVariantMap{}));
// -------------------------------------------------------------------------------------
     RM("newNode", { "Create new Node" },
        &BGCMS::newNode, ARG("basePath"), ARG<QVariantMap>("data"));
// -------------------------------------------------------------------------------------
     RM("updateNode", { "Update node" },
        &BGCMS::updateNode, ARG("node"), ARG<QVariantMap>("data"));
// -------------------------------------------------------------------------------------
     RM("removeNode", { "Remove node" },
        &BGCMS::removeNode, ARG("node"));
// =====================================================================================
     RM("copyNode", { "Copy node" },
          &BGCMS::copyNode, ARG("node"), ARG("target"), ARG<bool>("ref", false));
// -------------------------------------------------------------------------------------
     RM("copyRefNode", { "Copy reference node" },
       &BGCMS::copyRefNode, ARG("node"), ARG("target"));
// -------------------------------------------------------------------------------------
     RM("moveNode", { "Move node to new" },
          &BGCMS::moveNode, ARG("node"), ARG<int>("target"));
// -------------------------------------------------------------------------------------
     RM("nodePath", { "Get node path" },
          &BGCMS::nodePath, ARG("node"));
// =====================================================================================
    // clang-format on
}

ObjectInterface*
create(int, char**) {
    return new BGCMS;
}
