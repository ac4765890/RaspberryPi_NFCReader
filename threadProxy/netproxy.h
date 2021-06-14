#ifndef NETPROXY_H
#define NETPROXY_H

#include "judgement.h"
#include "common.h"
#include <QObject>
#include <QThread>
#include <QString>

class NetProxy : public QObject
{
    Q_OBJECT
public:
    explicit NetProxy(QObject *parent = nullptr, Data *_setting = nullptr);
    ~NetProxy();

signals:
    void sendStatus(const Status &sta);
    void sendMessage(const QString &text);
    void sendID(const QString &id);
    void sendOpenDoor();

public slots:


private:
    QThread workThread;
    Judgement *judge;
    Data *setting;

};

#endif // NETPROXY_H
