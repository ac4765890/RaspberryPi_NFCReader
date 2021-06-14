#ifndef SUPERVISION_H
#define SUPERVISION_H

#include "common.h"
#include <QObject>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>

class Supervision : public QObject
{
    Q_OBJECT
public:
    explicit Supervision(QObject *parent = nullptr);

    bool isRest;

signals:
    void clearPreID();
    void sendMessage(const QString &text, const bool restore = true);
    void sendStatus(const Status &sta);

public slots:
    void resetCardReader();
    void setIsRest();
    void startTimer();

private:
    QTimer *resetCardWD; //WD = WatchDog, if the id string is not empty, the program will not accept new one
                         //usually the string will clear by netdeal thread

    void stopTimer();
};

#endif // SUPERVISION_H
