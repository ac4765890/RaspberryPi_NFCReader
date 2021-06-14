#ifndef DATA_H
#define DATA_H

#include "common.h"
#include <QSettings>
#include <QStringRef>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QMutex>
#include <QMutexLocker>
#include <QDir>
#include <QApplication>
#include <QTimer>
#include <QDateTime>
#include <QObject>
#include <QSharedPointer>

#define KEY_SERVER_ADDRESS "serverAddress"
#define KEY_MACHINE_ADDRESS "machinAddress"
#define KEY_IMAGE_404 "image404way"
#define KEY_IMAGE_201 "image201way"
#define KEY_IMAGE_500 "image500way"
#define KEY_IMAGE_LOGO "imageLogoWay"
#define IDlist(x) (QString("uid/%1").arg(x))
#define BanList(x) (QString("ban/%1").arg(x))
#define UidDivers "uidDrivers"
#define serverEnable "serverEnable"

class Data : public QObject
{
    Q_OBJECT
public:
    enum class Member{fileway, serverAddress, machinAddress, uid, ban,
                      image_201, image_404, image_500, image_logo};
    enum class StringOperate{append, replace, del};
    static Data* getInstance(const QString &file);
    ~Data();

    void readFromFile();
    void writeToFile();
    void checkModify();
    void changeMember(const Member &me, const QString &text);
    void changeMember(const Member &me, const QStringList &text, const StringOperate &opr);

    const QString &_fileWay = filePath;
    const QString &_serverAddress = serverAddress;
    const QString &_machinAddress = machinAddress;
    const QString &_image_404 = image_404;
    const QString &_image_500 = image_500;
    const QString &_image_201 = image_201;
    const QString &_image_logo = image_logo;
    const QStringList &_uid = uid;
    const QStringList &_ban = ban;
    const bool &_serverFlag = serverFlag;
    const bool &_mfrc522 = mfrc522;

private:
    Data(const QString &file);
    static Data *instance;

    QSharedPointer<QSettings> profile;
    QSharedPointer<QFile> file;

    QString filePath;
    QString serverAddress;
    QString machinAddress;
    QString image_404;
    QString image_500;
    QString image_201;
    QString image_logo;
    QStringList uid;
    QStringList ban;
    QTimer fileCheckTimer;
    bool mfrc522;
    bool serverFlag;

    QDateTime lastModifyTime;
};

#endif // DATA_H
