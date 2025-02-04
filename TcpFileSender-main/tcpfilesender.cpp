﻿#include "tcpfilesender.h"

TcpFileSender::TcpFileSender(QWidget *parent)
    : QDialog(parent)
{
    loadSize = 1024 * 4;
    totalBytes = 0;
    bytesWritten = 0;
    bytesToWrite = 0;

    // 新增輸入框
    ipInput = new QLineEdit;
    portInput = new QLineEdit;
    ipInput->setPlaceholderText(QStringLiteral("輸入 IP 地址 (例如: 127.0.0.1)"));
    portInput->setPlaceholderText(QStringLiteral("輸入端口 (例如: 16998)"));
    portInput->setValidator(new QIntValidator(1, 65535, this)); // 限制端口為數字範圍

    clientProgressBar = new QProgressBar;
    clientStatusLabel = new QLabel(QStringLiteral("客戶端就緒"));
    startButton = new QPushButton(QStringLiteral("開始"));
    quitButton = new QPushButton(QStringLiteral("退出"));
    openButton = new QPushButton(QStringLiteral("開檔"));

    startButton->setEnabled(false);
    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(openButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    // 建立佈局
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(ipInput);
    mainLayout->addWidget(portInput);
    mainLayout->addWidget(clientProgressBar);
    mainLayout->addWidget(clientStatusLabel);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(10);
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(QStringLiteral("(版本控制Git管理)檔案傳送"));

    // 信號與槽的連接
    connect(openButton, SIGNAL(clicked()), this, SLOT(openFile()));
    connect(startButton, SIGNAL(clicked()), this, SLOT(start()));
    connect(&tcpClient, SIGNAL(connected()), this, SLOT(startTransfer()));
    connect(&tcpClient, SIGNAL(bytesWritten(qint64)), this, SLOT(updateClientProgress(qint64)));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
}

void TcpFileSender::openFile()
{
    fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) startButton->setEnabled(true);
}

void TcpFileSender::start()
{
    QString ip = ipInput->text();
    QString port = portInput->text();

    if (ip.isEmpty() || port.isEmpty())
    {
        QMessageBox::warning(this, QStringLiteral("錯誤"), QStringLiteral("請輸入有效的 IP 和端口！"));
        return;
    }

    startButton->setEnabled(false);
    bytesWritten = 0;
    clientStatusLabel->setText(QStringLiteral("連接中..."));

    tcpClient.connectToHost(QHostAddress(ip), port.toUInt());
}

void TcpFileSender::startTransfer()
{
    localFile = new QFile(fileName);
    if (!localFile->open(QFile::ReadOnly))
    {
        QMessageBox::warning(this, QStringLiteral("應用程式"),
                             QStringLiteral("無法讀取 %1:\n%2.").arg(fileName)
                                 .arg(localFile->errorString()));
        return;
    }

    totalBytes = localFile->size();
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_6);
    QString currentFile = fileName.right(fileName.size() -
                                         fileName.lastIndexOf("/") - 1);
    sendOut << qint64(0) << qint64(0) << currentFile;
    totalBytes += outBlock.size();

    sendOut.device()->seek(0);
    sendOut << totalBytes << qint64((outBlock.size() - sizeof(qint64) * 2));
    bytesToWrite = totalBytes - tcpClient.write(outBlock);

    clientStatusLabel->setText(QStringLiteral("已連接"));
    qDebug() << currentFile << totalBytes;
    outBlock.resize(0);
}

void TcpFileSender::updateClientProgress(qint64 numBytes)
{
    bytesWritten += (int)numBytes;
    if (bytesToWrite > 0)
    {
        outBlock = localFile->read(qMin(bytesToWrite, loadSize));
        bytesToWrite -= (int)tcpClient.write(outBlock);
        outBlock.resize(0);
    }
    else
    {
        localFile->close();
    }

    clientProgressBar->setMaximum(totalBytes);
    clientProgressBar->setValue(bytesWritten);
    clientStatusLabel->setText(QStringLiteral("已傳送 %1 Bytes").arg(bytesWritten));
}

TcpFileSender::~TcpFileSender()
{
}
