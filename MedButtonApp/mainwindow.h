#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include "QSqlError"
#include "markermodel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void readData();
    bool init_port();

private:
    Ui::MainWindow *ui;
    QSerialPort port;
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
    QSqlQueryModel* model = new QSqlQueryModel();
    MarkerModel marker_model;
};
#endif // MAINWINDOW_H
