#include "netproxy.h"

NetProxy::NetProxy(QObject *parent, Data *_setting) : QObject(parent), setting(_setting)
{
    judge = new Judgement(nullptr, setting);
    judge->moveToThread(&workThread);
    connect(this, &NetProxy::sendID, judge, &Judgement::recvID, Qt::DirectConnection);
    connect(judge, &Judgement::sendMessage, this, &NetProxy::sendMessage);
    connect(judge, &Judgement::sendStatus, this, &NetProxy::sendStatus);
    connect(&workThread, &QThread::finished, judge, &Judgement::deleteLater);
}

NetProxy::~NetProxy()
{
    workThread.quit();
    workThread.wait();
}
