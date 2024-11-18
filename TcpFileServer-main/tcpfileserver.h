#ifndef TCPFILESERVER_H
#define TCPFILESERVER_H

#include <QDialog>
#include <QtNetwork>
#include <QtWidgets>

class TcpFileServer : public QDialog
{
    Q_OBJECT

public:
    TcpFileServer(QWidget *parent = 0);
    ~TcpFileServer();
public slots:
    void start();
    void acceptConnection();
    void updateServerProgress();
    void displayError(QAbstractSocket::SocketError socketError);
private:
    QLineEdit *ipInput;    // 用於輸入 IP 的輸入框
    QLineEdit *portInput;  // 用於輸入 Port 的輸入框
    QProgressBar *serverProgressBar;
    QLabel *serverStatusLabel;
    QPushButton *startButton;
    QPushButton *quitButton;
    QDialogButtonBox *buttonBox;

    QFile *localFile;
    QTcpServer tcpServer;
    QTcpSocket *tcpServerConnection;
    qint64 totalBytes;
    qint64 byteReceived;
    qint64 fileNameSize;
    QByteArray inBlock;
    QString fileName;


};

#endif // TCPFILESERVER_H
