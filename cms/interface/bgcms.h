#ifndef BGCMS_H
#define BGCMS_H

#include <objectinterface.h>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

#include "BGCMS_global.h"

using namespace NS_BGMRPCObjectInterface;

class t_retBase {
public:
    t_retBase() = default;
    t_retBase(bool ok);
    t_retBase(const QString& error);

    bool ok() const;
    QString error() const;

    operator QVariant() const;
    virtual const QVariantMap toMap() const = 0;

    const QVariantMap& attr() const;
    QVariantMap& attr();

protected:
    void insertAttrs(QVariantMap& map) const;

protected:
    bool m_ok = false;
    QString m_error{ "null node" };
    QVariantMap m_attributes;
};

class t_retNode : public t_retBase {
public:
    t_retNode();
    t_retNode(const QVariantMap& node);
    t_retNode(const QString& error);

    QVariant ID() const;
    int nID() const;
    void setID(int id);
    QVariant pID() const;
    int nPID() const;

    const QVariantMap toMap() const;

    const QVariantMap& node() const;
    QVariantMap& node();

    QVariant operator[](const QString& key) const;
    QVariant& operator[](const QString& key);

private:
    QVariantMap m_node;
};

class t_retList : public t_retBase {
public:
    t_retList();
    t_retList(const QVariantList& list);
    t_retList(const QString& error);

    const QVariantMap toMap() const;

    const QVariantList& list() const;
    QVariantList& list();

private:
    QVariantList m_list;
};

class BGCMSAPP_EXPORT BGCMS : public ObjectInterface {
    Q_OBJECT
public:
    explicit BGCMS(QObject* parent = nullptr);

    // interfaces
    QString getCallerToken(QPointer<Caller> caller);

    QVariant node(QPointer<Caller> caller, const QVariantList& args);
    QVariant refNode(QPointer<Caller> caller, const QVariantList& args);
    QVariant nodeInfo(QPointer<Caller> caller, const QVariantList& args);
    QVariant refNodeInfo(QPointer<Caller> caller, const QVariantList& args);
    QVariant exists(QPointer<Caller> caller, const QVariantList& args);

    QVariant CNode(QPointer<Caller> caller, const QVariantList& args);
    QVariant CRefNode(QPointer<Caller> caller, const QVariantList& args);

    QVariant search(QPointer<Caller> caller, const QVariantMap& query,
                    int filter);
    QVariant list(QPointer<Caller> caller, const QVariant& pNode, int filter,
                  QVariantMap query);

    QVariant newNode(QPointer<Caller> caller, const QVariant& dir,
                     const QVariantMap& data);
    QVariant updateNode(QPointer<Caller> caller, const QVariant& target,
                        const QVariantMap& data);
    QVariant removeNode(QPointer<Caller> caller, const QVariant& target);

    QVariant copyNode(QPointer<Caller> caller, const QVariant& source,
                      const QVariant& target, bool ref);
    QVariant copyRefNode(QPointer<Caller> caller, const QVariant& source,
                         const QVariant& target);
    QVariant moveNode(QPointer<Caller> caller, const QVariant& source,
                      int target);

    QVariant nodePath(QPointer<Caller> caller, const QVariant& node);

private: /* --------------- node(token, ...) -------------------- */
    t_retNode node(const QString& token, const QVariantList& args);
    t_retNode node(const QString& token, int id);  // by id
    t_retNode node(const QString& token,           // by pnode/name
                   QVariant pNodeId, const QString& name, bool ref = true);
    t_retNode node(const QString& token,  // by path(relative path)
                   const QString& path, const t_retNode& pNode = {});

    /* ---------------------- search list -----------------------*/
    t_retList search(QPointer<Caller> caller, const QString& token,
                     const QVariantMap& query, int filter);

    t_retNode refNode(const QString& token, const QVariantList& args);

    /* ------------------- common ---------------------*/
    QVariantList getRows(QSqlQuery& query);
    t_retNode rowData(const QSqlRecord& record);

    QVariantMap updateStatementFragments(  // clang-format off
        const QStringList& fields, QVariantMap data,
        std::function<QVariant(const QString&, const QStringList&,
                               const QVariantMap&)> special);
                                           // clang-format on
    QVariantMap insertStatementFragments(  // clang-format off
        const QVariantMap& initData, const QVariantMap& data,
        std::function<QVariant(const QString&, const QVariantMap&,
                               const QVariantMap&)> special);
    // clang-format on

    bool isNumber(const QVariant& data) const;

protected:
    bool initial(int, char**) override;
    void registerMethods() override;

    QMutex m_BGCMSMutex;

    QSqlDatabase m_db{ QSqlDatabase::addDatabase("QSQLITE") };

    const QVariantMap m_rootNode{ { "pid", QVariant() },
                                  { "id", QVariant() },
                                  { "title", "/" },
                                  { "type", "D" } };

    /* ----------- sql fragment ---------------- */
    const QString infoFields{
        "`pid`, `id`, `type`, `title`, `name`, `contentType`, `summary`, "
        "`extData`, json_extract(:AL, \"$.\"||`own`) AS `own`, `hide`, "
        "`private`, `date`, `mdate`, `seq` "
    };
};

extern "C" {
NS_BGMRPCObjectInterface::ObjectInterface* create(int, char**);
}

#endif  // BGCMS_H
