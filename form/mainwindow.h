#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "common.h"
#include "data.h"
#include "threadproxy.h"
#include <cstdlib>
#include <QMainWindow>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QMessageBox>
#include <QPalette>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QPainter>
#include <QFont>
#include <QRectF>
#include <QSpacerItem>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>
#include <QMetaType>
#include <QLinkedList>
#include <QMovie>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void getScreenSize(int &x, int &y);
    void recvMessage(const DisplayMessage &message);

private:
    Ui::MainWindow *ui;
    bool isGetXml;
    int x, y, messageLabHight;
    Data *setting;
    ThreadProxy *thread;

    QVBoxLayout *mainVbx;

    QWidget *mainWid;
    QLabel *imageLab;
    QLabel *messageLab;
    QPixmap *picture;
    QTimer *resetCounter;
    QMutex bufferMutex;

    QLinkedList<const DisplayMessage> buffer;


    void setTextColor(const QColor &color);
    void setBackgroundColor(const QColor &color);
    void setPicture(const DisplayMessage::ImageType type);
    void updateScreen();
    const DisplayMessage getMessage();

protected:
    void keyPressEvent(QKeyEvent *event);
};

#endif // MAINWINDOW_H
