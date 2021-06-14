#include "supervision.h"

Supervision::Supervision(QObject *parent) : QObject(parent)
{
    resetCardWD = new QTimer(this);
    connect(resetCardWD, &QTimer::timeout, [this](){
        this->stopTimer();
        emit clearPreID();
        qDebug() << "class Supervision > trigger reader watch dog";
    });
}

void Supervision::resetCardReader()
{
    this->startTimer();
}

void Supervision::setIsRest()
{
    this->stopTimer(); 
    emit clearPreID();
}

void Supervision::stopTimer()
{
    qDebug() << "class Supervision > stop reader watch dog now";
    static QMutex mutex;
    QMutexLocker locker(&mutex);
    if (resetCardWD->isActive())
        resetCardWD->stop();
}

void Supervision::startTimer()
{
    qDebug() << "class Supervision > start reader watch dog now";
    if (resetCardWD->isActive())
        return;
    resetCardWD->start(30*1000);
}

