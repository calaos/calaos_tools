#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore>
#include <QtGui>
#include <QtWidgets>
#include <QtNetwork>
#include <QModbusServer>

namespace Ui {
class MainWindow;
}

#define MAX_DI  64
#define MAX_DO  64
#define MAX_AI  16
#define MAX_AO  16

#define WAGO_BCAST_PORT 4545
#define WAGO_UDP_PORT   4646

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void handleDeviceError(QModbusDevice::Error newError);
    void modbusDataWritten(QModbusDataUnit::RegisterType table, int address, int size);
    void udpDataReady();

    void on_checkBox_toggled(bool checked);
    void on_pushButton_clicked();

private:
    void inputDiPressed(int var, bool state);
    void inputDoPressed(int var, bool state);
    void inputAiChanged(int var, QString value);
    void inputAoChanged(int var, QString value);

    Ui::MainWindow *ui;

    QModbusServer *modbusDevice = nullptr;
    QUdpSocket *udpListenSocket = nullptr;

    QList<QPushButton *> listDI;
    QList<QCheckBox *> listDO;
    QList<QLineEdit *> listAI;
    QList<QLineEdit *> listAO;

    QString calaosServerIp;
};

#endif // MAINWINDOW_H
