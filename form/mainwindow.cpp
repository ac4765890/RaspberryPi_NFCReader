#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<UidOper>("UidOper");
    qRegisterMetaType<ServerRequest>("ServerRequest");
    qRegisterMetaType<ServerRespone>("ServerRespone");
    qRegisterMetaType<DisplayMessage>("DisplayMessage");
    system("tput civis > /dev/tty1");
    getScreenSize(x, y);
    this->setFixedSize(x, y);
    this->setWindowFlags(Qt::CustomizeWindowHint);

    picture = nullptr;
    resetCounter = new QTimer();
    connect(resetCounter, &QTimer::timeout, this, &MainWindow::updateScreen);

    mainWid = new QWidget(this);
    this->setCentralWidget(mainWid);

    messageLab = new QLabel(tr("Please place your ID card"), this);
    messageLab->setAlignment(Qt::AlignCenter);
    messageLab->setAutoFillBackground(true);
    //set message font size
    QFont ft(messageLab->font());
    ft.setPixelSize(40);
    messageLab->setFont(ft);
    //set message color
    setTextColor(QColor("green"));
    //set lab hight
    messageLabHight = messageLab->height()*2;
    messageLab->setMaximumHeight(messageLabHight);

    imageLab = new QLabel(this);
    imageLab->setAutoFillBackground(true);
    imageLab->setMinimumHeight(y-messageLabHight);
    imageLab->setAlignment(Qt::AlignCenter);
    imageLab->setUpdatesEnabled(true);

    setBackgroundColor(QColor("black"));

    mainVbx = new QVBoxLayout();
    mainVbx->setSpacing(0);
    mainVbx->setContentsMargins(0, 0, 0, 0);
    mainVbx->addWidget(imageLab);
    mainVbx->addWidget(messageLab);
    mainWid->setLayout(mainVbx);

    setting = Data::getInstance(DATA_FILE);
    picture = new QPixmap(setting->_image_logo);
    imageLab->setPixmap(*picture);
    imageLab->show();

    try{
        thread = nullptr;
        thread = new ThreadProxy(nullptr, setting);
    } catch(const char *str){
        qWarning() << str;
    }

    if (thread != nullptr){
        connect(thread, &ThreadProxy::sendMessage, this, &MainWindow::recvMessage);
    }

    resetCounter->start(100);
}

MainWindow::~MainWindow()
{
    delete setting;
    delete resetCounter;
    delete ui;
}

void MainWindow::getScreenSize(int &x, int &y)
{
    QDesktopWidget* desktopWidget = QApplication::desktop();
    QRect screenRect = desktopWidget->screenGeometry();
    x = screenRect.width();
    y = screenRect.height();
    delete desktopWidget;
}

void MainWindow::recvMessage(const DisplayMessage &message)
{
    QMutexLocker locker(&bufferMutex);

    if (message.clearAll){
        buffer.clear();
        buffer.push_back(message);
        return;
    }

    if (message.insertFirst){
        buffer.push_front(message);
        return;
    }

    buffer.push_back(message);
}

const DisplayMessage MainWindow::getMessage()
{
    QMutexLocker locker(&bufferMutex);
    return buffer.takeFirst();
}

void MainWindow::updateScreen()
{
    static quint16 preMessageResetTime = 0;
    static quint16 timeOutCounter = 0;

    if (preMessageResetTime <= timeOutCounter){
        if(buffer.isEmpty()){
            if (preMessageResetTime == 0)
                return;

            preMessageResetTime = timeOutCounter = 0;
            messageLab->setText(tr("Please place your ID card"));
            setBackgroundColor(QColor(tr("black")));
            setPicture(DisplayMessage::ImageType::imageLogo);
            setTextColor(QColor(tr("green")));
            return;
        }

        const DisplayMessage &message = getMessage();

        timeOutCounter = 0;
        preMessageResetTime = message.resetTimeMs/100;
        messageLab->setText(message.message);
        setBackgroundColor(message.backGroundColor);
        setTextColor(message.fontColor);
        setPicture(message.image);
    } else {
        timeOutCounter++;
    }

}

void MainWindow::setBackgroundColor(const QColor &color)
{
#ifdef NSS_MACHINE_DEBUG
    QPalette *pal = new QPalette(messageLab->palette());
    pal->setColor(QPalette::Background, "red");
    messageLab->setPalette(*pal);
    pal->setColor(QPalette::Background, "green");
    imageLab->setPalette(*pal);

#else
    QPalette *pal = new QPalette(messageLab->palette());
    pal->setColor(QPalette::Background, color);
    imageLab->setPalette(*pal);
    messageLab->setPalette(*pal);
#endif
}

void MainWindow::setPicture(const DisplayMessage::ImageType type)
{
    if (picture != nullptr){
        delete picture;
    }
    switch (type){
    case DisplayMessage::ImageType::image201:
        picture = new QPixmap(setting->_image_201);
        break;
    case DisplayMessage::ImageType::image404:
        picture = new QPixmap(setting->_image_404);
        break;
    case DisplayMessage::ImageType::image500:
        picture = new QPixmap(setting->_image_500);
        break;
    case DisplayMessage::ImageType::imageLogo:
        picture = new QPixmap(setting->_image_logo);
        break;
    }


    imageLab->setPixmap(picture->scaledToHeight(y-messageLabHight));
    imageLab->show();
    imageLab->repaint();
}

void MainWindow::setTextColor(const QColor &color)
{
    QPalette *pal = new QPalette(messageLab->palette());
    pal->setColor(QPalette::WindowText, color);
    messageLab->setPalette(*pal);
    delete pal;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F10){
        const QMessageBox::StandardButton &reply = QMessageBox::question(this, tr("Info"), tr("Are you sure you want to quit?"), QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
            QApplication::exit();
    }
}
