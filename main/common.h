#ifndef COMMON_H
#define COMMON_H

#ifdef QT_NO_DEBUG
#define NSS_MACHINE_RELEASE
#else
#define NSS_MACHINE_DEBUG
#endif

#define DATA_FILE "/data/data.ini"
#define SERVER_URL "http://172.24.5.150/dooros"
#define SERVER_PORT "23333"
#define IMAGE_404_PATH "/data/404.jpg"
#define IMAGE_201_PATH "/data/201.jpg"
#define IMAGE_500_PATH "/data/500.jpg"
#define IMAGE_LOGO_PATH "/data/logo.jpg"
#define RESET_TIME_MS 2000
#define CHECK_FILE_TIME_MS  5000
#define SERVER_REQUEST_TIMEOUT_MS  4000
#define READER_EN_GPIO 8
#define RELAY_GPIO 23
#define GPIO_EXPORT_PATH "/sys/class/gpio/export"
#define GPIO_UNEXPORT_PATH "/sys/class/gpio/unexport"
#define GPIO_DIRECTION_PATH(x) QString("/sys/class/gpio/gpio%1/direction").arg(x)
#define GPIO_VALUE_PATH(x) QString("/sys/class/gpio/gpio%1/value").arg(x)

#include <QDebug>
#include <QString>
#include <QColor>

struct DisplayMessage{
    enum class ImageType{image404, image201, image500, imageLogo};

    QColor backGroundColor;
    QColor fontColor;
    ImageType image;
    QString message;
    quint16 resetTimeMs;
    bool insertFirst;
    bool clearAll;
};

struct ServerRequest{
    enum class PackageType{judge, log, sync};

    PackageType type;
    QString uid;
    quint16 timeoutMs;
    bool insertFirst;
    bool clearAll;
};

struct ServerRespone{
    enum class PackageType{access, ban, unknow, lost};

    PackageType type;
};

struct UidOper{
    enum class Oper{del, add, delList, addList, ban, replaceUid, replaceBan, judge};

    Oper opr;
    QString uid;
};

#endif // COMMON_H
