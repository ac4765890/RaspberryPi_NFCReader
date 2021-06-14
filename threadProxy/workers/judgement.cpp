#include "judgement.h"

DisplayMessage Judgement::access = {
    .backGroundColor = QColor("black"),
    .fontColor = QColor("green"),
    .image = DisplayMessage::ImageType::imageLogo,
    .message = tr("Welcome"),
    .resetTimeMs = 2*1000,
    .insertFirst = false,
    .clearAll = false
};

DisplayMessage Judgement::ban = {
    .backGroundColor = QColor("white"),
    .fontColor = QColor("green"),
    .image = DisplayMessage::ImageType::image201,
    .message = tr("This card had been ban"),
    .resetTimeMs = 4*1000,
    .insertFirst = false,
    .clearAll = false
};

DisplayMessage Judgement::unknow = {
    .backGroundColor = QColor("white"),
    .fontColor = QColor("green"),
    .image = DisplayMessage::ImageType::image404,
    .message = tr("This card didn't registered"),
    .resetTimeMs = 4*1000,
    .insertFirst = false,
    .clearAll = false
};

DisplayMessage Judgement::serverErr = {
    .backGroundColor = QColor("white"),
    .fontColor = QColor("green"),
    .image = DisplayMessage::ImageType::image500,
    .message = tr("Remote server panic"),
    .resetTimeMs = 4*1000,
    .insertFirst = false,
    .clearAll = false
};

DisplayMessage Judgement::waitServer = {
    .backGroundColor = QColor("black"),
    .fontColor = QColor("green"),
    .image = DisplayMessage::ImageType::imageLogo,
    .message = tr("Waiting server..."),
    .resetTimeMs = 2*1000,
    .insertFirst = true,
    .clearAll = false
};

DisplayMessage Judgement::serverDisable = {
    .backGroundColor = QColor("white"),
    .fontColor = QColor("green"),
    .image = DisplayMessage::ImageType::image500,
    .message = tr("Server Disable, please connect admin"),
    .resetTimeMs = 2*1000,
    .insertFirst = true,
    .clearAll = false
};

ServerRequest Judgement::reqLog = {
    .type = ServerRequest::PackageType::log,
    .uid = "",
    .timeoutMs = 1*1000,
    .insertFirst = false,
    .clearAll = false
};

ServerRequest Judgement::reqJudge = {
    .type = ServerRequest::PackageType::judge,
    .uid = "",
    .timeoutMs = 4*1000,
    .insertFirst = true,
    .clearAll = false
};

Judgement::Judgement(QObject *parent, Data *_setting) : QObject(parent), setting(_setting)
{
    uidClearFlag = false;
    AMfrc522::gpioMount();
    AMfrc522::gpioHandle(RELAY_GPIO, true);
    net = new NetDeal(this, setting);
    setting->changeMember(Data::Member::machinAddress, net->getIP());
    connect(net, &NetDeal::sendID, this, &Judgement::recvID);
    connect(net, &NetDeal::sendRespone, this, &Judgement::recvServerResult);
    connect(this, &Judgement::sendServerJudgeRequest, net, &NetDeal::recvServerRequest);
    connect(&uidDDOS, &QTimer::timeout, [this](){
        if (!uidClearFlag && preUid.isEmpty())
            return;
        if (timeoutCount++ > 4){
            timeoutCount = 0;
            uidClearFlag = true;
        }
    });
    if (setting->_serverFlag){
        net->postIP(setting->_machinAddress);
        net->postGetIDList();
    }
    uidDDOS.start(1000);     //1s, 4s to timeout
}

Judgement::~Judgement()
{
    //delete net;
}

bool Judgement::localJudge(const QString &uid)
{
    if (setting->_uid.contains(uid)){
        judgeSuccess();
        return true;
    }
    if (setting->_ban.contains(uid)){
        judgeBan();
        return true;
    }

    if (setting->_serverFlag){
        reqLog.uid = uid;
        sendServerJudgeRequest(reqLog);
    } else {
        emit sendMessage(serverDisable);
    }

    return false;
}

void Judgement::recvIdDDOS(const UidOper oper)
{
    idLocker.lock();

    if (uidClearFlag){
        preUid.clear();
        uidClearFlag = false;
    }

    if (oper.uid != preUid){
        qDebug() << "class Judgement > recv ID: " << oper.uid;
        preUid = oper.uid;
        recvID(oper);
    }

    idLocker.unlock();
}

void Judgement::recvID(const UidOper &uid)
{
    const QStringList &list = uid.uid.split(',');
    switch(uid.opr){
    case UidOper::Oper::add:
    case UidOper::Oper::addList:
        setting->changeMember(Data::Member::uid, list, Data::StringOperate::append);
        setting->changeMember(Data::Member::ban, list, Data::StringOperate::del);
        break;
    case UidOper::Oper::ban:
        setting->changeMember(Data::Member::uid, list, Data::StringOperate::del);
        setting->changeMember(Data::Member::ban, list, Data::StringOperate::append);
        break;
    case UidOper::Oper::del:
    case UidOper::Oper::delList:
        setting->changeMember(Data::Member::uid, list, Data::StringOperate::del);
        setting->changeMember(Data::Member::ban, list, Data::StringOperate::del);
        break;
    case UidOper::Oper::replaceUid:
        setting->changeMember(Data::Member::uid, list, Data::StringOperate::replace);
        break;
    case UidOper::Oper::replaceBan:
        setting->changeMember(Data::Member::ban, list, Data::StringOperate::replace);
        break;
    case UidOper::Oper::judge:
        if (!localJudge(uid.uid) && setting->_serverFlag){
            qDebug() << "class Judgement > request server";
            reqJudge.uid = uid.uid;
            emit sendServerJudgeRequest(reqJudge);
            emit sendMessage(waitServer);
        }
        break;
    }
}

void Judgement::recvServerResult(const ServerRespone& respone)
{
    switch(respone.type){
    case ServerRespone::PackageType::access:
        judgeSuccess();
        break;
    case ServerRespone::PackageType::ban:
        judgeBan();
        break;
    case ServerRespone::PackageType::unknow:
        judgeUnknow();
        break;
    case ServerRespone::PackageType::lost:
        judgeServerLost();
        break;
    }
}

void Judgement::judgeSuccess()
{
    qDebug() << "class Judgement > judeg success";
    openDoor();
    emit sendMessage(access);
}

void Judgement::judgeBan()
{
    qDebug() << "class Judgement > judeg ban";
    emit sendMessage(ban);
}

void Judgement::judgeUnknow()
{
    qDebug() << "class Judgement > judeg unknow";
    emit sendMessage(unknow);
}

void Judgement::judgeServerLost()
{
    qDebug() << "class Judgement > serverLost";
    emit sendMessage(serverErr);
}


void Judgement::openDoor()
{
    AMfrc522::gpioWrite(RELAY_GPIO, 1);
    QThread::msleep(300);
    AMfrc522::gpioWrite(RELAY_GPIO, 0);
}
