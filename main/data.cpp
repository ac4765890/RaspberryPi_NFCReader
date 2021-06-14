#include "data.h"

Data::Data(const QString &_file) :QObject(nullptr)
{
    filePath = QApplication::applicationDirPath() + _file;
    file = QSharedPointer<QFile>::create(filePath);
    qDebug()<<"class Data > currentPath: "<< filePath;

    if (!file->exists()){
        qDebug() << "class Data > Warning: file not exists, now creating";

        QDir dir;
        QStringRef dirPath = filePath.leftRef(filePath.lastIndexOf("/"));
        dir.mkpath(dirPath.toString());
    }

    file->open(QIODevice::ReadWrite);
    try{
        if (!file->isOpen())
            throw "class Data: can not open file";
    } catch(const char *str){
        qCritical() << str;
    }
    file.reset();
    profile = QSharedPointer<QSettings>::create(filePath, QSettings::IniFormat);
    readFromFile();

    QFileInfo fileinfo(filePath);
    lastModifyTime = fileinfo.lastModified();

    connect(&fileCheckTimer, &QTimer::timeout, this, &Data::checkModify);
    fileCheckTimer.start(CHECK_FILE_TIME_MS);
}

Data::~Data()
{
    this->writeToFile();
}

void Data::checkModify()
{
    QFileInfo file(filePath);
    if (lastModifyTime != (file.lastModified())){
        qDebug() << "class Data > data file modify, reload";
        lastModifyTime = file.lastModified();
        readFromFile();
    }
}

void Data::readFromFile()
{
    profile = QSharedPointer<QSettings>::create(filePath, QSettings::IniFormat);
    this->serverAddress = profile->value(KEY_SERVER_ADDRESS, "http://172.24.5.150/dooros").toString();
    this->machinAddress = profile->value(KEY_MACHINE_ADDRESS, "").toString();
    this->image_201 = profile->value(KEY_IMAGE_201, ":/http/201").toString();
    this->image_404 = profile->value(KEY_IMAGE_404, ":/http/404").toString();
    this->image_500 = profile->value(KEY_IMAGE_500, ":/http/500").toString();
    this->image_logo = profile->value(KEY_IMAGE_LOGO, ":/http/logo").toString();
    this->mfrc522 = profile->value(UidDivers, "0").toBool();
    this-> serverFlag = profile->value(serverEnable, "0").toBool();
    int count = profile->value(IDlist(0), "0").toString().toInt();
    for (int i = 1; i <= count; i++){
        uid.append(profile->value(IDlist(i), "").toString());
        qDebug() << "class Data > append: " << uid.back();
    }

    count = profile->value(BanList(0), "0").toString().toInt();
    for (int i = 1; i <= count; i++)
        ban.append(profile->value(BanList(i), "").toString());

}

void Data::writeToFile()
{
    profile->setValue(KEY_SERVER_ADDRESS, serverAddress);
    profile->setValue(KEY_MACHINE_ADDRESS, machinAddress);
    profile->setValue(KEY_IMAGE_201, image_201);
    profile->setValue(KEY_IMAGE_404, image_404);
    profile->setValue(KEY_IMAGE_500, image_500);
    profile->setValue(KEY_IMAGE_LOGO, image_logo);
    profile->setValue(UidDivers, mfrc522 ? 1 : 0);
    profile->setValue(IDlist(0), uid.size());
    profile->setValue(serverEnable, serverFlag);
    for (int i = 1; i <= uid.size(); i++)
        profile->setValue(IDlist(i), uid.at(i-1));

    profile->setValue(BanList(0), ban.size());
    for (int i = 1; i <= ban.size(); i++)
         profile->setValue(BanList(i), ban.at(i-1));

    profile->sync();
}

void Data::changeMember(const Member &me, const QString &text)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    switch (me){
        case Member::fileway:
            filePath = text;
            break;
        case Member::serverAddress:
            serverAddress = text;
            break;
        case Member::machinAddress:
            machinAddress = text;
            break;
        case Member::image_201:
            image_201 = text;
            break;
        case Member::image_404:
            image_404 = text;
            break;
        case Member::image_500:
            image_500 = text;
            break;
        case Member::image_logo:
            image_logo = text;
            break;

        default:
            qWarning() << "class Data: you use the wrong member function";
    }
}

void Data::changeMember(const Member &me, const QStringList &text, const StringOperate &opr)
{
    static QMutex mutex;
    QMutexLocker locker(&mutex);

    auto append = [](const QStringList &origin, QStringList &target){
        qDebug() << "class Date > append date now";
        for (int i = 0; i < origin.size(); i++)
            target.append(origin.at(i).toUpper());
    };
    auto replace = [](const QStringList &origin, QStringList &target){
        qDebug() << "class Date > replace date now";
        target.clear();
        for (int i = 0; i < origin.size(); i++)
            target.append(origin.at(i).toUpper());
    };
    auto del = [](const QStringList &origin, QStringList &target){
        qDebug() << "class Date > del date now";
        for (int i = 0; i < origin.size(); i++){
            int index;
            while ((index = target.indexOf(origin.at(i).toUpper())) != -1){
                target.removeAt(index);
            }
        }
    };

    auto fun = [append, replace, del](const QStringList &origin, QStringList &target, const StringOperate &opr){
        switch (opr){
        case StringOperate::append:
            append(origin, target);
            break;
        case StringOperate::del:
            del(origin, target);
            break;
        case StringOperate::replace:
            replace(origin, target);
            break;
        }
    };

    switch (me) {
    case Member::ban:
        fun(text, ban, opr);
        break;
    case Member::uid:
        fun(text, uid, opr);
        break;
    default:
        qWarning() << "class Data: you use the wrong member function";
    }
    this->writeToFile();
    QFileInfo file(filePath);
    this->lastModifyTime = file.lastModified();
}

Data* Data::instance = nullptr;

Data* Data::getInstance(const QString &file)
{
    if (instance == nullptr){
        static QMutex mutex;
        QMutexLocker locker(&mutex);
        if (instance == nullptr){
            instance = new Data(file);
        }
    }
    return instance;
}
