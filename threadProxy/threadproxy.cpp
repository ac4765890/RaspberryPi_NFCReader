 #include "threadproxy.h"

ThreadProxy::ThreadProxy(QObject *parent, Data *setting) : QObject(parent)
{
    try {
        mfrcWorker = new AMfrc522();
    } catch (const char *str){
        throw str;
    }
    judgeWorker = new Judgement(nullptr, setting);

    mfrcWorker->moveToThread(&mfrcWorkThread);
    connect(this, &ThreadProxy::disableDriver, mfrcWorker, &AMfrc522::disableDriver);
    connect(this, &ThreadProxy::enableDriver, mfrcWorker, &AMfrc522::enableDriver);
    connect(&mfrcWorkThread, &QThread::finished, mfrcWorker, &AMfrc522::deleteLater);

    judgeWorker->moveToThread(&judgeWorkThread);
    connect(mfrcWorker, &AMfrc522::sendID, judgeWorker, &Judgement::recvID, Qt::DirectConnection);
    connect(mfrcWorker, &AMfrc522::sendIdDDOS, judgeWorker, &Judgement::recvIdDDOS, Qt::DirectConnection);
    connect(judgeWorker, &Judgement::sendMessage, this, &ThreadProxy::sendMessage);
    connect(&judgeWorkThread, &QThread::finished, judgeWorker, &Judgement::deleteLater);
    connect(&mfrcWorkThread, &QThread::started, mfrcWorker, &AMfrc522::startWork);

    mfrcWorkThread.start();
    judgeWorkThread.start();
}

ThreadProxy::~ThreadProxy()
{
    mfrcWorkThread.quit();
    judgeWorkThread.quit();
    mfrcWorkThread.wait();
    judgeWorkThread.wait();
}
