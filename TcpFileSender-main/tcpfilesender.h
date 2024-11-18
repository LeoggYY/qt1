#ifndef TCPFILESENDER_H
#define TCPFILESENDER_H

#include <QDialog>
#include <QTcpSocket>
#include <QFile>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QDataStream>
#include <QHostAddress>
#include <QIntValidator>
#include <QSettings>
#include <QDebug>

class TcpFileSender : public QDialog
{
    Q_OBJECT

public:
    TcpFileSender(QWidget *parent = 0);
    ~TcpFileSender();
public slots:
    void start();
    void startTransfer();
    void updateClientProgress(qint64 numBytes);
    void openFile();
private:
    QLineEdit *ipInput;    // 用於輸入 IP 的輸入框
    QLineEdit *portInput;  // 用於輸入 Port 的輸入框
    QProgressBar *clientProgressBar;
    QLabel *clientStatusLabel;
    QPushButton *startButton;
    QPushButton *quitButton;
    QPushButton *openButton;
    QDialogButtonBox *buttonBox;

    QFile *localFile;
    QTcpSocket tcpClient;
    qint64 totalBytes;
    qint64 bytesWritten;
    qint64 bytesToWrite;
    qint64 loadSize;
    QString fileName;
    QByteArray outBlock;
};

#endif // TCPFILESENDER_H
