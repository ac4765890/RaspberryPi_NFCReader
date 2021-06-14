#include "netdeal.h"

const ServerRespone NetDeal::serverLost = {
  .type = ServerRespone::PackageType::lost
};

NetDeal::NetDeal(QObject *parent, Data *_setting) : QObject(parent), setting(_setting)
{
    serverLoop = new QTimer(this);
    serverRespone = true;

    qRegisterMetaType<UidStatus>("UidStatus");

    server = new JQHttpServer::TcpServerManage(2);
    server->setHttpAcceptedCallback([this](const QPointer< JQHttpServer::Session > &session){
        qDebug() << "class NetDeal > HttpBody: " << session->requestBody();
        sendXml(QString::fromStdString(session->requestBody().toStdString()));
        session->replyText("");
    });

    serverLoop->start(1000);
    connect(serverLoop, &QTimer::timeout, this, &NetDeal::serverBufferCheck);
    qDebug() << "class NetDeal > listen:" << server->listen(QHostAddress::Any, QString(SERVER_PORT).toInt());
}

NetDeal::~NetDeal()
{
    delete server;
}

const ServerRequest NetDeal::getRequest()
{
    QMutexLocker lock(&buffLocker);
    return buffer.takeFirst();
}

void NetDeal::serverBufferCheck()
{
    QMutexLocker lock(&serverLocker);
    static quint16 timeoutCount = 0;

    if (!buffer.isEmpty()){
        const ServerRequest &request = getRequest();
        switch (request.type){
        case ServerRequest::PackageType::judge:
            switchPost(XMLType::openDoor, QStringList(request.uid));
            break;
        case ServerRequest::PackageType::log:
            switchPost(XMLType::postLog, QStringList(request.uid));
            break;
        case ServerRequest::PackageType::sync:
            switchPost(XMLType::getUid);
            break;
        }
    }

    if (timeoutCount >= 3600){
        timeoutCount = 0;
        postGetIDList();
    } else {
        timeoutCount++;
    }
}

void NetDeal::recvServerRequest(const ServerRequest &request)
{
    QMutexLocker lock(&buffLocker);

    if (request.insertFirst){
        buffer.push_front(request);
        return;
    }
    if (request.clearAll){
        buffer.clear();
        buffer.push_back(request);
        return;
    }
    buffer.push_back(request);
}

void NetDeal::postLog(const QString &uid)
{
    switchPost(XMLType::postLog, QStringList(uid));
}

void NetDeal::postOpen(const QString &uid)
{
    switchPost(XMLType::openDoor,  QStringList(uid));
}

void NetDeal::postGetIDList()
{
    switchPost(XMLType::getUid);
}

void NetDeal::postIP(const QString &url)
{
    switchPost(XMLType::postIP, QStringList("http://" + url + ":" + SERVER_PORT));
}

QString NetDeal::getIP()
{
    QString ip;
    QList<QHostAddress> AddressList = QNetworkInterface::allAddresses();
    foreach(QHostAddress address, AddressList){
        if(address.protocol() == QAbstractSocket::IPv4Protocol &&
                address != QHostAddress::Null &&
                address != QHostAddress::LocalHost){
            if (address.toString().contains("192.168.")){
                ip = address.toString();
                break;
            }
        }
    }
    qDebug() << "class NetDeal > machine address " << ip;
    return ip;
}

void NetDeal::switchPost(const XMLType &type, const QStringList &value)
{
    QDomDocument xml(constructXML(type));
    switch(type){
    case XMLType::openDoor:
    case XMLType::postLog:
        setXmlElementText(xml, QStringList("uid"), value);
        break;
    case XMLType::getUid:
        break;
    case XMLType::postIP:
        setXmlElementText(xml,  QStringList("ip"), value);
        break;
    }
    qDebug() << "class NetDeal > construct XML: " << xml.toString();
    constructPost(xml.toString(), QUrl(setting->_serverAddress));

}

QDomDocument NetDeal::constructXML(const XMLType &type)
{
    QDomDocument doc;
    QDomElement root(doc.createElement("nss"));
    doc.appendChild(root);

    QDomElement child(doc.createElement("type"));
    root.appendChild(child);

    QDomText *text = nullptr;
    switch(type){
        case XMLType::openDoor:
        case XMLType::postLog:{
            switch (type){
                case XMLType::openDoor:
                    text = new QDomText(doc.createTextNode("opendoor"));
                break;
                case XMLType::postLog:
                    text = new QDomText(doc.createTextNode("postLog"));
                break;
                default:
                break;
            }
            QDomElement child(doc.createElement("uid"));
            root.appendChild(child);
            break;
        }
        case XMLType::getUid:
            text = new QDomText(doc.createTextNode("getuid"));
        break;
        case XMLType::postIP:{
            text = new QDomText(doc.createTextNode("updateip"));
            QDomElement child(doc.createElement("ip"));
            root.appendChild(child);
            break;
        }
    }

    try{
        if (text == nullptr)
            throw "class NetDeal > class NetDeal: unkonow type";
        child.appendChild(*text);
        delete text;
    } catch (const char *str){
        qCritical() << str;
    }
    qDebug() << "class NetDeal > construct XML: " << doc.toString();
    return doc;
}

void NetDeal::setXmlElementText(QDomDocument &doc, const QStringList &element, const QStringList &text)
{
    try{
        if (element.size() != text.size())
            throw "class NetDeal: class NetDeal: element count is not same as text";
    } catch(const char *str){
        qCritical() << str;
        return;
    }

    for (int i = 0; i < element.size(); i++) {
        QDomNodeList nodeList = doc.firstChild().childNodes();
        QString te(element.at(i));
        for (int j = 0; j < nodeList.count(); j++){
            QDomElement temp(nodeList.item(j).toElement());
            if (temp.tagName() == te){
                QDomText te(doc.createTextNode(text.at(i)));
                temp.appendChild(te);
                qDebug() << "class NetDeal > success to change text";
                break;
            }
        }
    }
}


void NetDeal::constructPost(const QString &_xml, const QUrl &url)
{
    QString xml;
    for (int i = 0; i < _xml.size(); i++)
        if (_xml[i] != '\n')
            xml += _xml[i];
    qDebug() << "class NetDeal > deal xml: " << xml;

    QNetworkAccessManager manager;
    connect(&manager, &QNetworkAccessManager::finished, this, &NetDeal::postFinish);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/octet-stream");
    request.setRawHeader("filename", QString("upload.xml").toUtf8());

    QByteArray arr(xml.toLatin1());

    qDebug() << "class NetDeal > buff: " << arr;

    manager.post(request, arr);

    QEventLoop loop;
    connect(&manager, &QNetworkAccessManager::finished, &loop, &QEventLoop::quit);
    loop.exec();
    qDebug() << "class NetDeal > alread post";
}

void NetDeal::postFinish(QNetworkReply *re)
{
    //qDebug() << "class NetDeal > post success: " << re->readAll();
    if (re->error() != QNetworkReply::NoError){
        qDebug() << "class NetDeal > post has error: " << re->error();
        postError(re->error());
    }
}

void NetDeal::postError(QNetworkReply::NetworkError re)
{
    emit sendRespone(serverLost);
}

void NetDeal::sendXml(const QString &_xml)
{
    QString xml;
    for (int i = 0; i < _xml.size(); i++)
        if (_xml[i] != '\n' && _xml[i] != '\t')
            xml += _xml[i];

    qDebug() << "class NetDeal > switch xml: " << xml;
    QXmlStreamReader reader(xml);
    bool isAppend = false;
    while (!reader.atEnd()){
        QXmlStreamReader::TokenType token = reader.readNext();
        if (isAppend){
            isAppend = false;
            QString text = reader.text().toString();

            if (text == "opendoor")
                getDoorSingal(xml);
            else if (text == "getuid")
                getIDList(xml);
            else if (text == "deluid")
                getOperate(xml, UidStatus::del);
            else if (text == "adduid")
                getOperate(xml, UidStatus::add);
            else if (text == "banuid")
                getOperate(xml, UidStatus::ban);
        }
        if (token == QXmlStreamReader::StartElement){
            if (reader.name().toString() == "type")
                isAppend = true;
        }
    }
}

void NetDeal::getIDList(const QString &xml)
{
    qDebug() << "class Netdeal > recv IDlist xml: " << xml;
    static UidOper uid;
    uid.uid.clear();
    uid.opr = UidOper::Oper::replaceUid;

    QXmlStreamReader reader(xml);
    bool isAppend = false;
    while (!reader.atEnd()){
         QXmlStreamReader::TokenType token = reader.readNext();
         if (isAppend){
             isAppend = false;
             if (!reader.text().toString().isEmpty()){
                 uid.uid.append(reader.text().toString());
                 uid.uid.append(",");
                 qDebug() << "class Judgement > idList append text: " << reader.text().toString();
             }
         }
         if (token == QXmlStreamReader::StartElement){
             if (reader.name() == "uid"){
                 isAppend = true;
             }
         }
    }
    sendID(uid);
}

void NetDeal::getDoorSingal(const QString &xml)
{
    qDebug() << "class Judgement > recv DoorSingal xml: " << xml;

    ServerRespone respone;
    QXmlStreamReader reader(xml);
    bool isAppend = false;
    while (!reader.atEnd()){
        QXmlStreamReader::TokenType token = reader.readNext();
        if (isAppend){
            QString text = reader.text().toString();
            if (text == "success"){
                respone.type = ServerRespone::PackageType::access;
                break;
            } else if (text == "ban"){
                respone.type = ServerRespone::PackageType::ban;
                break;
            } else if (text == "unknow"){
                respone.type = ServerRespone::PackageType::unknow;
                break;
            }
            isAppend = false;
        }
        if (token == QXmlStreamReader::StartElement)
            if (reader.name() == "status")
                isAppend = true;
    }
    sendRespone(respone);
}

void NetDeal::getOperate(const QString &xml, const NetDeal::UidStatus &sta)
{
    qDebug() << "class Judgement > recv opr xml: " << xml;

    static UidOper uid;
    uid.uid.clear();
    QXmlStreamReader reader(xml);
    bool isAppend = false;
    QString temp = "uid";
    while (!reader.atEnd()){
        QXmlStreamReader::TokenType token = reader.readNext();
        if (isAppend){
            isAppend = false;
            switch (sta) {
            case NetDeal::UidStatus::add:
                qDebug() << "class Judgement > add uid";
                uid.opr = UidOper::Oper::add;
                uid.uid = reader.text().toString();
                break;
            case NetDeal::UidStatus::del:
                qDebug() << "class Judgement > del uid";
                uid.opr = UidOper::Oper::del;
                uid.uid = reader.text().toString();
                break;
            case NetDeal::UidStatus::ban:
                qDebug() << "class Judgement > ban uid";
                uid.opr = UidOper::Oper::ban;
                uid.uid = reader.text().toString();
                break;
            default:
                qWarning() << "class Judgement: unknow enum type";
            }

        }
        if (token == QXmlStreamReader::StartElement){
            if (reader.name().toString() == temp )
                isAppend = true;
        }
    }
    sendID(uid);
}
