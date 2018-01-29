#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QModbusTcpServer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    for (int i = MAX_DI - 1;i >= 0;i--)
    {
        QPushButton *b = new QPushButton(ui->groupBoxDI);
        b->setText(tr("Input %1").arg(i));
        connect(b, &QPushButton::pressed, [=]()
        {
            if (!b->isCheckable())
                inputDiPressed(i, true);
        });
        connect(b, &QPushButton::released, [=]()
        {
            if (!b->isCheckable())
                inputDiPressed(i, false);
        });
        connect(b, &QPushButton::toggled, [=](bool state)
        {
            if (b->isCheckable())
                inputDiPressed(i, state);
        });
        ui->verticalLayout_6->insertWidget(1, b);
        listDI.prepend(b);
    }

    for (int i = MAX_DI - 1;i >= 0;i--)
    {
        QCheckBox *c = new QCheckBox(ui->groupBoxDO);
        c->setText(tr("Output %1").arg(i));
        connect(c, &QCheckBox::toggled, [=](bool state)
        {
            inputDoPressed(i, state);
        });
        ui->verticalLayout_7->insertWidget(0, c);
        listDO.prepend(c);
    }

    for (int i = MAX_AI - 1;i >= 0;i--)
    {
        QLineEdit *l = new QLineEdit(ui->groupBoxAI);
        l->setPlaceholderText("Hexadecimal A-F, a-f, 0-9.");
        l->setMaxLength(32767);
        ui->verticalLayout_8->insertWidget(0, l);
        connect(l, &QLineEdit::textChanged, [=](const QString &txt)
        {
            inputAiChanged(i, txt);
        });
        listAI.prepend(l);
    }

    for (int i = MAX_AO - 1;i >= 0;i--)
    {
        QLineEdit *l = new QLineEdit(ui->groupBoxAO);
        l->setPlaceholderText("Hexadecimal A-F, a-f, 0-9.");
        l->setMaxLength(32767);
        ui->verticalLayout_9->insertWidget(0, l);
        connect(l, &QLineEdit::textChanged, [=](const QString &txt)
        {
            inputAoChanged(i, txt);
        });
        listAO.prepend(l);
    }

    statusBar()->clearMessage();
    statusBar()->showMessage("Not running");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_checkBox_toggled(bool checked)
{
    for (int i = 0;i < listDI.size();i++)
    {
        listDI.at(i)->setCheckable(checked);
    }
}

void MainWindow::on_pushButton_clicked()
{
    statusBar()->clearMessage();
    if (modbusDevice)
    {
        //Stop
        modbusDevice->disconnect();
        delete modbusDevice;
        modbusDevice = nullptr;

        udpListenSocket->close();
        delete udpListenSocket;
        udpListenSocket = nullptr;

        ui->pushButton->setText("Start");
        statusBar()->showMessage("Not running");
        return;
    }

    modbusDevice = new QModbusTcpServer(this);

    QModbusDataUnitMap reg;
    reg.insert(QModbusDataUnit::Coils, { QModbusDataUnit::Coils, 0, MAX_DO });
    reg.insert(QModbusDataUnit::DiscreteInputs, { QModbusDataUnit::DiscreteInputs, 0, MAX_DI });
    reg.insert(QModbusDataUnit::InputRegisters, { QModbusDataUnit::InputRegisters, 0, MAX_AI });
    reg.insert(QModbusDataUnit::HoldingRegisters, { QModbusDataUnit::HoldingRegisters, 0, MAX_AO });

    modbusDevice->setMap(reg);

    connect(modbusDevice, &QModbusServer::dataWritten,
            this, &MainWindow::modbusDataWritten);
    connect(modbusDevice, &QModbusServer::errorOccurred,
            this, &MainWindow::handleDeviceError);

    for (int i = 0;i < listDI.size();i++)
        modbusDevice->setData(QModbusDataUnit::Coils, i, listDI.at(i)->isChecked());
    for (int i = 0;i < listDO.size();i++)
        modbusDevice->setData(QModbusDataUnit::DiscreteInputs, i, listDO.at(i)->isChecked());
    for (int i = 0;i < listAI.size();i++)
        modbusDevice->setData(QModbusDataUnit::InputRegisters, i, listAI.at(i)->text().toInt(nullptr, 16));
    for (int i = 0;i < listAO.size();i++)
        modbusDevice->setData(QModbusDataUnit::HoldingRegisters, i, listAO.at(i)->text().toInt(nullptr, 16));

    modbusDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter, 502);
    modbusDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, "0.0.0.0");
    modbusDevice->setServerAddress(1);
    if (!modbusDevice->connectDevice())
    {
        statusBar()->showMessage(tr("Connect failed: ") + modbusDevice->errorString(), 5000);
        delete modbusDevice;
        modbusDevice = nullptr;
        return;
    }

    udpListenSocket = new QUdpSocket(this);
    udpListenSocket->bind(QHostAddress::Any, WAGO_UDP_PORT, QAbstractSocket::ShareAddress);
    connect(udpListenSocket, &QUdpSocket::readyRead, this, &MainWindow::udpDataReady);

    ui->pushButton->setText("Stop");
    statusBar()->showMessage("Running...");
}

void MainWindow::handleDeviceError(QModbusDevice::Error newError)
{
    if (newError == QModbusDevice::NoError || !modbusDevice)
        return;

    statusBar()->showMessage(modbusDevice->errorString(), 5000);
}

void MainWindow::modbusDataWritten(QModbusDataUnit::RegisterType table, int address, int size)
{
    for (int i = 0; i < size; ++i)
    {
        quint16 value;
        QString text;
        switch (table)
        {
        case QModbusDataUnit::Coils:
            modbusDevice->data(QModbusDataUnit::Coils, address + i, &value);
            listDO.at(address + i)->setChecked(value);
            break;
        case QModbusDataUnit::HoldingRegisters:
            modbusDevice->data(QModbusDataUnit::HoldingRegisters, address + i, &value);
            listAO.at(address + i)->setText(text.setNum(value, 16));
            break;
        default:
            break;
        }
    }
}

void MainWindow::inputDiPressed(int var, bool state)
{
    QHostAddress addr(calaosServerIp);
    QString d = QStringLiteral("WAGO INT %1 %2")
                .arg(var)
                .arg(state);

    if (modbusDevice)
        modbusDevice->setData(QModbusDataUnit::DiscreteInputs, var, state);

    if (udpListenSocket)
    {
        qDebug() << "Sending " << d;
        udpListenSocket->writeDatagram(d.toLatin1(), addr, WAGO_BCAST_PORT);
        udpListenSocket->writeDatagram(d.toLatin1(), QHostAddress::Broadcast, WAGO_BCAST_PORT);
    }
}

void MainWindow::inputAiChanged(int var, QString value)
{
    if (modbusDevice)
        modbusDevice->setData(QModbusDataUnit::InputRegisters, var, value.toInt(nullptr, 16));
}

void MainWindow::inputAoChanged(int var, QString value)
{
    if (modbusDevice)
        modbusDevice->setData(QModbusDataUnit::HoldingRegisters, var, value.toInt(nullptr, 16));
}

void MainWindow::inputDoPressed(int var, bool state)
{
    if (modbusDevice)
        modbusDevice->setData(QModbusDataUnit::Coils, var, state);
}

void MainWindow::udpDataReady()
{
    while (udpListenSocket && udpListenSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpListenSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        udpListenSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QString data = QString::fromLatin1(datagram);

        qDebug() << "received: " << data;
        QString result;

        if (data == "WAGO_GET_OUTTYPE")
            result = data + " 0";
        else if (data == "WAGO_GET_OUTTADDR ")
            result = data + " 0 0 -1";
        else if (data.startsWith("WAGO_GET_INFO_MODULE "))
            result = data + " 0 0 0 0";
        else if (data.startsWith("WAGO_GET_INFO"))
            result = data + " 0 0 0 0 0 0 0";
        else if (data.startsWith("WAGO_DALI_GET "))
            result = "WAGO_DALI_GET 0 0 0";
        else if (data.startsWith("WAGO_DALI_GET_ADDR"))
        {
            result = data + " ";
            for (int i = 0;i < 64;i++)
                result += "0";
        }
        else if (data.startsWith("WAGO_DALI_GET_DEVICE_INFO "))
            result = data + " 0 0 0 0 0 0";
        else if (data.startsWith("WAGO_DALI_GET_DEVICE_GROUP "))
        {
            result = data + " ";
            for (int i = 0;i < 16;i++)
                result += "0";
        }
        else if (data.startsWith("WAGO_DALI_DEVICE_ADD_GROUP "))
            result = "WAGO_DALI_DEVICE_ADD_GROUP 1";
        else if (data.startsWith("WAGO_DALI_DEVICE_DEL_GROUP "))
            result = "WAGO_DALI_DEVICE_DEL_GROUP 1";
        else if (data.startsWith("WAGO_DALI_ADDRESSING_STATUS "))
            result = "WAGO_DALI_ADDRESSING_STATUS 1";
        else if (data.startsWith("WAGO_GET_VERSION"))
            result = data + " 2.0 750-849";
        else if (data.startsWith("WAGO_INFO_VOLET_GET "))
            result = data + " 0";
        else if (data.startsWith("WAGO_SET_SERVER_IP "))
            calaosServerIp = data.remove("WAGO_SET_SERVER_IP ");
        else if (data.startsWith("WAGO_SET_OUTPUT"))
        {
            QStringList sl = data.split(' ');
            if (sl.count() >= 3)
            {
                int var = sl.at(1).toInt();
                bool state = sl.at(2).toInt();

                if (var >= 0 && var < listDO.count())
                    listDO.at(var)->setChecked(state);
            }
        }

        if (!result.isEmpty())
        {
            qDebug() << "Sending: " << result;
            udpListenSocket->writeDatagram(result.toLatin1(), sender, senderPort);
        }
    }
}
