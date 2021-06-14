#include "amfrc522.h"
#include <cstdio>

bool AMfrc522::gpioInit = false;

nfc_device *AMfrc522::pnd = NULL;
nfc_context *AMfrc522::context;


AMfrc522::AMfrc522(QObject *parent, bool _mfrc522) : QObject(parent), mfrc522(_mfrc522)
{
    mfrc = nullptr;
    uid.opr = UidOper::Oper::judge;
    uid.uid = "";
    if (mfrc522){
        mfrc = new MFRC522();
        mfrc->PCD_Init();
        gpioMount();
        gpioHandle(READER_EN_GPIO, true);
        gpioWrite(READER_EN_GPIO, 1);
    } else {
        try{
            nfcInit();
        } catch(const char *str){
            throw str;
        }
    }
}

AMfrc522::~AMfrc522()
{
    if (mfrc522){
        delete mfrc;
    } else {
        nfc_close(pnd);
        nfc_exit(context);
    }
}

void AMfrc522::startWork()
{
#define NFC_CARDTYPE_SIZE       5
    auto toAscii = [](byte &in){
        if (in <= 0x09)
            in += 48;
        else
            in += 55;
    };

    if(mfrc522){
        qDebug() << "class AMfrc522 > mfrc work now";

        while (1){
            while (1){
                if (!mfrc->PICC_IsNewCardPresent())
                    continue;

                if (!mfrc->PICC_ReadCardSerial())
                    continue;
            }

            byte group[mfrc->uid.size*2];
            for (byte i = 0; i < mfrc->uid.size; i += 2){
                byte temp = (mfrc->uid.uidByte[i] >> 4) & 0x0f;
                toAscii(temp);
                group[i] = temp;

                temp = mfrc->uid.uidByte[i] & 0x0f;
                toAscii(temp);
                group[i+1] = temp;
            }

            //don't have uid check
            uid.uid = QString((char *)group);
            sendID(uid);
            qDebug() << "class AMfrc522 > card id: " << uid.uid;
        }
    } else {
        try{
            qDebug() << "class AMfrc522 > nfc work now";
            auto fun = [&](const uint8_t *pbtData, const size_t szBytes)->QString{
                QString re;
                for (size_t i = 0; i < szBytes; i++){
                    byte temp = (pbtData[i] >> 4) & 0x0f;
                    toAscii(temp);
                    re += temp;

                    temp = pbtData[i] & 0x0f;
                    toAscii(temp);
                    re += temp;
                }
                return re;
            };

            const nfc_modulation nmMifare[NFC_CARDTYPE_SIZE] = {
                { .nmt = NMT_ISO14443A, .nbr = NBR_106 },
                { .nmt = NMT_ISO14443B, .nbr = NBR_106 },
                { .nmt = NMT_FELICA, .nbr = NBR_212 },
                { .nmt = NMT_FELICA, .nbr = NBR_424 },
                { .nmt = NMT_JEWEL, .nbr = NBR_106 },
            };

            //try to read card type
            while (1){
                nfc_target nt;
                int res;
                for (int i = 0; i < NFC_CARDTYPE_SIZE; i++){
                    if ((res = nfc_initiator_select_passive_target(pnd, nmMifare[i], NULL, 0, &nt) ) > 0) {
                        const QString &re = fun(nt.nti.nai.abtUid, nt.nti.nai.szUidLen);
                        if (!re.isEmpty()){
                            uid.uid = re;
                            emit sendIdDDOS(uid);
                            break;
                        }
                    }
                }
            }
        } catch(const char *str){
            if (mfrc != nullptr){
                delete mfrc;
                throw str;
            }
            if (!mfrc522){
                nfcInit();
            }
        }
    }
    qDebug() << "class AMfrc522 > now end of fun";
}

void AMfrc522::nfcInit()
{
    nfc_init(&context);
    if (context == NULL) {
      throw "FATAL ERROR: Unable to init libnfc (malloc)";

    }

    const char *acLibnfcVersion = nfc_version();
    qWarning("libnfc %s\n", acLibnfcVersion);
    pnd = nfc_open(context, NULL);
    if (pnd == NULL) {
      throw "FATAL ERROR: Unable to open NFC device.";
    }

    if (nfc_initiator_init(pnd) < 0) {
      nfc_perror(pnd, "nfc_initiator_init");
      throw "FATAL ERROR: nfc device init error";
    }

    qDebug("class AMfrc522 > NFC reader: %s opened\n", nfc_device_get_name(pnd));
}

void AMfrc522::disableDriver()
{
    gpioWrite(READER_EN_GPIO, 0);
}

void AMfrc522::enableDriver()
{
    gpioWrite(READER_EN_GPIO, 1);
}

bool AMfrc522::gpioMount()
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    if (gpioInit)
        return true;
    else
        gpioInit = true;
    wiringPiSetupGpio();
    return true;

    /*QFile *file = new QFile(GPIO_EXPORT_PATH);
    if (!file->open(QIODevice::WriteOnly)){
        qWarning() << "class AMfrc522: open export file failed";
        return false;
    }

    if (file->write(QString("%1/n").arg(gpio).toLatin1()) == -1){
        qWarning() << "class AMfrc522: write export file failed";
        return false;
    }

    file->waitForBytesWritten(5000);
    file->close();
    delete file;
    return true;*/
}

void AMfrc522::gpioHandle(int pin, bool isOut)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    pinMode(pin, isOut ? OUTPUT : INPUT);
    /*QFile *file = new QFile(GPIO_DIRECTION_PATH(READER_EN_GPIO));
    bool isExists = false;
    for (int tryCount = 0; tryCount < 5; tryCount++){
        QThread::msleep(100);
        if (file->exists()){
            isExists = true;
            break;
        }
    }

    if (!file->open(QIODevice::WriteOnly)){
        qWarning() << "class AMfrc522: open direction file failed";
        return false;
    }

    if (isExists){
        if (file->write(QString("%1/n").arg(isOut ? "out" : "in").toLatin1()) == -1){
            qWarning() << "class AMfrc522: write direction file failed";
            return false;
        }
    } else {
        qWarning() << "class AMfrc522: direction file not exists";
        return false;
    }
    file->waitForBytesWritten(5000);
    delete file;
    return true;*/
}

/*bool AMfrc522::gpioUnmount(int gpio)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    QFile file(GPIO_UNEXPORT_PATH);
    if (!file.open(QIODevice::WriteOnly)){
        qWarning() << "class AMfrc522: open unexport file failed";
        return false;
    }

    if (file.write(QString("%1/n").arg(gpio).toLatin1()) == -1){
        qWarning() << "class AMfrc522: write unexport file failed";
        return false;
    }
    file.waitForBytesWritten(5000);
    file.close();
    return true;
}*/

void AMfrc522::gpioWrite(int gpio, bool status)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    digitalWrite(gpio, status ? 1 : 0);
    /*QFile file(GPIO_VALUE_PATH(gpio));
    if (!file.exists()){
        qWarning() << "class AMfrc522: value file not exists";
        return false;
    }

    if (file.write(QString("%1/n").arg(status ? 1 : 0).toLatin1()) == -1){
        qWarning() << "class AMfc522: write value file failed";
        return false;
    }
    file.waitForBytesWritten(5000);
    return true;*/
}
