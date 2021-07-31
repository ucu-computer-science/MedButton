#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "markermodel.h"
#include <QDebug>
#include <QQuickView>
#include <QQuickItem>
#include <QVBoxLayout>
#include <QQmlContext>
#include "stdlib.h"
#include <regex>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qDebug() << "Bruh!";

    /* Serial port */
    init_port();
    connect(&port, &QSerialPort::readyRead, this, &MainWindow::readData);

    /* Database */
    // воно не створить само базу даних, треба її мати у себе на комп'ютері. Якщо її нема - крашнеться
    // Я потім пропишу створення
    // ПС Може бути помилка з тим, що воно не бачить postgres, а бачить лише sql .... уже забула який
    //    Якщо буде така, напишіть - я скину бібліотеки, які треба покласти біля виконавчого файлу
    db.setHostName("localhost");
    db.setDatabaseName("medbutton");
    db.setPassword("postgres");
    db.setUserName("postgres");

    if(db.open()) {
        QSqlQuery* query = new QSqlQuery(db);
        query->prepare("SELECT * FROM Soldiers");
        query->exec();
        model->setQuery(*query);
     } else {
        qDebug() << db.lastError().text();
        exit(1);
     }


    /* Map */
    QQuickView *qvMap = new QQuickView();

    marker_model.setMaxMarkers(10); // не знаю чи треба, можливо добре мати, щоб вся карта в мітках не була
    qvMap->rootContext()->setContextProperty("marker_model", &marker_model);
    qvMap->setSource(QUrl("qrc:/mapview.qml"));
    QWidget *qvMapContainer = QWidget::createWindowContainer(qvMap);
    QVBoxLayout *lay = new QVBoxLayout(ui->centralwidget);
    lay->addWidget(qvMapContainer);

    // тут просто добавляла мітки для тесту
//    marker_model.moveMarker(QGeoCoordinate(24.0464, 59.0428));
//    marker_model.moveMarker(QGeoCoordinate(20.0464, 59.0428));
//    marker_model.moveMarker(QGeoCoordinate(15.4561, 73.8021));
//    marker_model.moveMarker(QGeoCoordinate(10.4561, 73.8021));
//    marker_model.moveMarker(QGeoCoordinate(24.0464, 73.0428));
//    marker_model.change(4, QGeoCoordinate(20.0464, 10.0428));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readData()
{
    const QByteArray data = port.readLine();
    std::string str(data.constData(), data.length());
    // я тут поміняла regex, але тестувала чи він працює лише на сайті, а не в коді :(
    // 21:15:10-49.8234343,23.02423443\r\n <- можливий вигляд (перше тире між годиною і координатами, якщо є друге це вже від'ємна координата)
    static const std::regex r(R"((\d{2}:){2}\d{2}--?\d+.\d+,-?\d+.\d+\r\n)");
    if(std::regex_match(str.data(), r)) {
        qDebug() << QString::fromUtf8(str.c_str());
        std::string lat = str.substr(str.find_first_of("-") + 1, str.find_first_of(",") - str.find_first_of("-") - 1);
        std::string lon = str.substr(str.find_first_of(",") + 1, str.find_first_of('\r') - str.find_first_of(",") - 1);
        qDebug() << std::stof(lat);
        qDebug() << std::stof(lon);
        marker_model.moveMarker(QGeoCoordinate(std::stof(lat), std::stof(lon)));
    }
}

bool MainWindow::init_port()
{
    /* Set up serial port */
    port.setPortName("COM9");
    if (port.open(QSerialPort::OpenModeFlag::ReadWrite)) {
        port.setBaudRate(QSerialPort::Baud115200);
        port.setDataBits(QSerialPort::Data8);
        port.setStopBits(QSerialPort::OneStop);
        port.setParity(QSerialPort::NoParity);
        port.setFlowControl(QSerialPort::NoFlowControl);
        qDebug() << "Port opened";
    } else {
        qDebug() << "Error opening";
        return false;
    }

    /* Connect gprs */
    qDebug() << port.write("AT\r");
    if (port.waitForReadyRead(1000)) {
        qDebug() << "Error reading";
        return false;
    }
    if ("AT\r\r\nOK\r\n" != port.readAll()) {
        qDebug() << "Error conecting gprs";
        return false;
    }

    qDebug() << port.write("AT+CMGF=1\r");
    port.waitForReadyRead(1000);
    if ("AT+CMGF=1\r\r\nOK\r\n" != port.readAll()) {
        qDebug() << "Error conecting gprs";
        return false;
    }

    qDebug() << port.write("AT+CNMI=1,2,0,0,0\r");
    port.waitForReadyRead(1000);
    if ("AT+CNMI=1,2,0,0,0\r\r\nOK\r\n" != port.readAll()) {
        qDebug() << "Error conecting gprs";
        return false;
    }

    return true;
}

