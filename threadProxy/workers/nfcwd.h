#ifndef NFCWD_H
#define NFCWD_H

#include <QObject>

class nfcWD : public QObject
{
    Q_OBJECT
public:
    explicit nfcWD(QObject *parent = nullptr);

signals:

public slots:
};

#endif // NFCWD_H