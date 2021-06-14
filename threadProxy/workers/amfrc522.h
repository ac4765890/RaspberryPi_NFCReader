#ifndef AMFRC522_H
#define AMFRC522_H

#include "common.h"
#include "MFRC522.h"
#include "nfc.h"
#include "wiringPi.h"
#include <cstdlib>
#include <QObject>
#include <QString>
#include <QMutex>
#include <QMutexLocker>
#include <QFile>
#include <QThread>
#include <QCoreApplication>

class AMfrc522 : public QObject
{
    Q_OBJECT
public:
    explicit AMfrc522(QObject *parent = nullptr, bool _mfrc522 = false);
    ~AMfrc522();

    static bool gpioMount();
    //static bool gpioUnmount(int gpio);
    static void gpioHandle(int pin, bool isOut);
    static void gpioWrite(int gpio, bool status);

signals:
    void sendID(const UidOper &uid);
    void sendIdDDOS(const UidOper &uid);

public slots:
    void disableDriver();
    void enableDriver();
    void startWork();

private:
    static bool gpioInit;
    MFRC522 *mfrc;
    bool mfrc522;
    UidOper uid;

    static nfc_device *pnd;
    static nfc_context *context;

    void nfcInit();
    void nfcWork();
};

#endif // AMFRC522_H
