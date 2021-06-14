#ifndef ThreadProxy_H
#define ThreadProxy_H

#include "common.h"
#include "MFRC522.h"
#include "data.h"
#include "amfrc522.h"
#include "judgement.h"
#include <QObject>
#include <QThread>
#include <QString>

class ThreadProxy : public QObject
{
    Q_OBJECT
public:
    explicit ThreadProxy(QObject *parent = nullptr, Data *setting = nullptr);
    ~ThreadProxy();

signals:
    void disableDriver();
    void enableDriver();
    void sendMessage(const DisplayMessage &message);

private:
    QThread mfrcWorkThread;
    QThread judgeWorkThread;
    AMfrc522 *mfrcWorker;
    Judgement *judgeWorker;
};

#endif // ThreadProxy_H
