#ifndef NETDEAL_H
#define NETDEAL_H

#include "common.h"
#include "data.h"
#include "JQHttpServer.h"
#include <QTimer>
#include <QMetaType>
#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QString>
#include <QStringList>
#include <QNetworkInterface>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QDomDocument>
#include <QDomElement>
#include <QDomText>
#include <QXmlStreamReader>
#include <QFile>
#include <QTextStream>
#include <QEventLoop>
#include <QLinkedList>
#include <QMutex>

class NetDeal : public QObject
{
    Q_OBJECT
public:
    enum class UidStatus{add, del, ban};
    explicit NetDeal(QObject* parent = nullptr, Data *_setting = nullptr);
    ~NetDeal();

    QString getIP();
    void postGetIDList();
    void postIP(const QString &url);

public slots:
    void recvServerRequest(const ServerRequest &request);

signals:
    void sendID(const UidOper &uid);
    void sendRespone(const ServerRespone &respone);

private:
    enum class XMLType{openDoor, getUid, postIP, postLog};
    JQHttpServer::TcpServerManage *server;
    QDomDocument constructXML(const XMLType &);
    Data *setting;
    QTimer *serverLoop; 
    QLinkedList<const ServerRequest> buffer;
    QMutex buffLocker;
    QMutex serverLocker;
    bool serverRespone;

    static const ServerRespone serverLost;


    const ServerRequest getRequest();
    void serverBufferCheck();
    void setXmlElementText(QDomDocument &doc, const QStringList &element, const QStringList &text);
    void constructPost(const QString &xml, const QUrl &url);
    void switchPost(const XMLType &type, const QStringList &value =  QStringList(""));

    void postFinish(QNetworkReply *re);
    void sendXml(const QString &xml);
    void stopTimer();
    void startTimer();
    void postOpen(const QString &uid);
    void postLog(const QString &uid);
    void postError(QNetworkReply::NetworkError re);

    void getIDList(const QString &xml);
    void getDoorSingal(const QString &xml);
    void getOperate(const QString &xml, const NetDeal::UidStatus &sta);

};

#endif // NETDEAL_H
