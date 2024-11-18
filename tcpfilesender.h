#ifndef TCPFILESENDER_H
#define TCPFILESENDER_H

#include <QDialog>
#include <QtNetwork>
#include <QtWidgets>

class TcpFileSender : public QDialog
{
    Q_OBJECT

public:
    TcpFileSender(QWidget *parent = nullptr);
    ~TcpFileSender();

private slots:
    void openFile();               // 開檔方法
    void startTransfer();          // 開始傳送方法
    void updateClientProgress(qint64 numBytes); // 更新進度條
    void enableStartButton();      // 啟用開始按鈕的方法
    void start();                  // 開始傳送檔案

private:
    QLineEdit *ipLineEdit;
    QLineEdit *portLineEdit;
    QProgressBar *clientProgressBar;
    QLabel *clientStatusLabel;
    QPushButton *startButton;
    QPushButton *quitButton;
    QPushButton *openButton;
    QTcpSocket tcpClient;
    QFile *localFile;
    QString fileName;
    QByteArray outBlock;
    qint64 totalBytes;
    qint64 bytesWritten;
    qint64 bytesToWrite;
    int loadSize;
};

#endif // TCPFILESENDER_H
