#include "tcpfileserver.h"
#define tr QStringLiteral

TcpFileServer::TcpFileServer(QWidget *parent)
    : QDialog(parent)
{
    totalBytes = 0;
    byteReceived = 0;
    fileNameSize = 0;

    // 新增 IP 和 Port 輸入框
    ipInput = new QLineEdit;
    portInput = new QLineEdit;
    ipInput->setPlaceholderText(QStringLiteral("輸入伺服器 IP (預設: 127.0.0.1)"));
    portInput->setPlaceholderText(QStringLiteral("輸入伺服器 Port (預設: 16998)"));
    portInput->setValidator(new QIntValidator(1, 65535, this)); // 限制端口範圍
    ipInput->setText("127.0.0.1"); // 設定默認值
    portInput->setText("16998");

    serverProgressBar = new QProgressBar;
    serverStatusLabel = new QLabel(QStringLiteral("伺服器端就緒"));
    startButton = new QPushButton(QStringLiteral("接收"));
    quitButton = new QPushButton(QStringLiteral("退出"));
    buttonBox = new QDialogButtonBox;
    buttonBox->addButton(startButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(ipInput);    // 添加 IP 輸入框
    mainLayout->addWidget(portInput);  // 添加 Port 輸入框
    mainLayout->addWidget(serverProgressBar);
    mainLayout->addWidget(serverStatusLabel);
    mainLayout->addStretch();
    mainLayout->addWidget(buttonBox);
    setLayout(mainLayout);

    setWindowTitle(QStringLiteral("接收檔案"));

    // 信號與槽函數連接
    connect(startButton, SIGNAL(clicked()), this, SLOT(start()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(&tcpServer, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    connect(&tcpServer, SIGNAL(acceptError(QAbstractSocket::SocketError)), this,
            SLOT(displayError(QAbstractSocket::SocketError)));
}
void TcpFileServer::start()
{
    QString ip = ipInput->text();
    QString port = portInput->text();

    // 驗證 IP 和 Port
    QHostAddress address;
    if (!address.setAddress(ip)) {
        QMessageBox::warning(this, QStringLiteral("錯誤"), QStringLiteral("請輸入有效的 IP 地址！"));
        return;
    }

    bool ok;
    int portNumber = port.toInt(&ok);
    if (!ok || portNumber < 1 || portNumber > 65535) {
        QMessageBox::warning(this, QStringLiteral("錯誤"), QStringLiteral("請輸入有效的端口號（1-65535）！"));
        return;
    }

    startButton->setEnabled(false);
    byteReceived = 0;
    fileNameSize = 0;

    // 設置伺服器監聽
    while (!tcpServer.isListening() && !tcpServer.listen(address, portNumber)) {
        QMessageBox::StandardButton ret = QMessageBox::critical(
            this,
            QStringLiteral("錯誤"),
            QStringLiteral("無法啟動伺服器: %1.").arg(tcpServer.errorString()),
            QMessageBox::Retry | QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel) {
            startButton->setEnabled(true);
            return;
        }
    }

    serverStatusLabel->setText(QStringLiteral("監聽中..."));
}
TcpFileServer::~TcpFileServer()
{
    if (localFile) {
        localFile->close();
        delete localFile;
    }
}
void TcpFileServer::acceptConnection()
{
    // 這是函數的具體實現
    tcpServerConnection = tcpServer.nextPendingConnection();
    connect(tcpServerConnection, SIGNAL(readyRead()),
            this, SLOT(updateServerProgress()));
    connect(tcpServerConnection, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(displayError(QAbstractSocket::SocketError)));
    serverStatusLabel->setText(tr("接受連線"));
    tcpServer.close();
}
void TcpFileServer::updateServerProgress()
{
    QDataStream in(tcpServerConnection);
    in.setVersion(QDataStream::Qt_4_6);

    if (byteReceived <= sizeof(qint64) * 2) {
        if ((fileNameSize == 0) && (tcpServerConnection->bytesAvailable() >= sizeof(qint64) * 2)) {
            in >> totalBytes >> fileNameSize;
            byteReceived += sizeof(qint64) * 2;
        }
        if ((fileNameSize != 0) && (tcpServerConnection->bytesAvailable() >= fileNameSize)) {
            in >> fileName;
            byteReceived += fileNameSize;
            localFile = new QFile(fileName);
            if (!localFile->open(QFile::WriteOnly)) {
                QMessageBox::warning(this, tr("應用程式"),
                                     tr("無法打開檔案 %1：\n%2.")
                                         .arg(fileName)
                                         .arg(localFile->errorString()));
                return;
            }
        } else {
            return;
        }
    }

    if (byteReceived < totalBytes) {
        byteReceived += tcpServerConnection->bytesAvailable();
        inBlock = tcpServerConnection->readAll();
        localFile->write(inBlock);
        inBlock.resize(0);
    }

    serverProgressBar->setMaximum(totalBytes);
    serverProgressBar->setValue(byteReceived);

    serverStatusLabel->setText(tr("已接收 %1 Bytes").arg(byteReceived));

    if (byteReceived == totalBytes) {
        tcpServerConnection->close();
        localFile->close();
        serverStatusLabel->setText(tr("接收完成"));
    }
}

