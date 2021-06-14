#ifndef JUDGEMENT_H
#define JUDGEMENT_H

#include "common.h"
#include "netdeal.h"
#include "data.h"
#include "amfrc522.h"
#include <QObject>
#include <QXmlStreamReader>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>

class Judgement : public QObject
{
    Q_OBJECT
public:
    explicit Judgement(QObject *parent = nullptr, Data *_setting = nullptr);
    ~Judgement();

signals:
    void sendMessage(const DisplayMessage &message);
    void sendServerJudgeRequest(const ServerRequest request);

public slots:
    void recvID(const UidOper &oper);
    void recvIdDDOS(const UidOper oper);
    void recvServerResult(const ServerRespone& respone);

private:
    Data *setting;
    NetDeal *net;
    QString judgeID;
    QMutex idLocker;
    QTimer uidDDOS;
    quint32 timeoutCount;
    QString preUid;
    bool uidClearFlag;

    static DisplayMessage access;
    static DisplayMessage ban;
    static DisplayMessage unknow;
    static DisplayMessage serverErr;
    static DisplayMessage waitServer;
    static DisplayMessage serverDisable;
    static ServerRequest reqJudge;
    static ServerRequest reqLog;

    bool localJudge(const QString &uid);
    void judgeSuccess();
    void judgeBan();
    void judgeUnknow();
    void judgeServerLost();
    void openDoor();
};

#endif // JUDGEMENT_H
