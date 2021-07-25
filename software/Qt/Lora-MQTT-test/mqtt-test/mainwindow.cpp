#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->txt_box->setText("Test text!");

    manager = new QNetworkAccessManager();
    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(managerFinished(QNetworkReply*)));

    // Example application name - "first-random-app"
    // limit=1 - return only 1 (most recent) LoRa message
    request.setUrl(QUrl("https://eu1.cloud.thethings.network/api/v3/as/applications/first-random-app/packages/storage/uplink_message?limit=1"));
    // API Key goes after "Bearer"
    request.setRawHeader("Authorization" , "Bearer NNSXS.DJBXJQNTIBAYD6LR5NVPUNP64EKDH54OLFKQGSI.OODUM6B6NVTPRDHJIDLUEWMU6IY3JG35Z34OWJ5EMMUVQS5XUQRA");
    request.setRawHeader("Accept" , "text/event-stream");
    request.setRawHeader("Content-Type" , "text/event-stream");

    manager->get(request);
}

void MainWindow::managerFinished(QNetworkReply *reply)
{
    QVariant status_code = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if (status_code.isValid())
    {
        QString status = status_code.toString();
        QByteArray rawResponse = reply->readAll();
        QJsonDocument jsonResponse = QJsonDocument::fromJson(rawResponse);
        QJsonObject jsonObject = jsonResponse.object();
        QString jsonData = jsonObject.value("result").toObject().value("uplink_message").toObject().value("frm_payload").toString();
        /* decoded from Base64 data will be stored in uplinkMsgText */
        QString uplinkMsgText = QByteArray::fromBase64(jsonData.toUtf8());
        qDebug() << "Received from server LoRa uplink_message: " << uplinkMsgText;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_btn_test_clicked()
{
}
