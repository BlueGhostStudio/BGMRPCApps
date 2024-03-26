#ifndef GRAPHVIZ_H
#define GRAPHVIZ_H

#include "Graphviz_global.h"
#include <objectinterface.h>

using namespace  NS_BGMRPCObjectInterface;

class GRAPHVIZ_EXPORT Graphviz : public ObjectInterface {
public:
    Graphviz(QObject* parent = nullptr);

    QVariant render(QPointer<Caller> caller, const QByteArray& data);

protected:
    void registerMethods() override;
};

extern "C" {
NS_BGMRPCObjectInterface::ObjectInterface* create(int, char**);
}

#endif  // GRAPHVIZ_H
