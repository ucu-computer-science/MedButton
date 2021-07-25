#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "QtNetwork/QtNetwork"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QNetworkAccessManager *manager;
    QNetworkRequest request;

private slots:
    void managerFinished(QNetworkReply *reply);
    void on_btn_test_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
