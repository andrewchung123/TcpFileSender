#include "tcpfilesender.h"

TcpFileSender::TcpFileSender(QWidget *parent)
    : QDialog(parent)
{
    loadSize = 1024 * 4;
    totalBytes = 0;
    bytesWritten = 0;
    bytesToWrite = 0;

    // 初始化元件
    clientProgressBar = new QProgressBar;
    clientStatusLabel = new QLabel(QStringLiteral("客戶端就緒"));
    startButton = new QPushButton(QStringLiteral("開始"));
    quitButton = new QPushButton(QStringLiteral("退出"));
    openButton = new QPushButton(QStringLiteral("開檔"));
    ipLineEdit = new QLineEdit;
    portLineEdit = new QLineEdit;

    // 設置提示文字
    ipLineEdit->setPlaceholderText("輸入 IP 位址");
    portLineEdit->setPlaceholderText("輸入 Port");
    startButton->setEnabled(false);

    // 佈局設定
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QGridLayout *inputLayout = new QGridLayout;

    inputLayout->addWidget(new QLabel(QStringLiteral("IP 位址：")), 0, 0);
    inputLayout->addWidget(ipLineEdit, 0, 1);
    inputLayout->addWidget(new QLabel(QStringLiteral("Port：")), 1, 0);
    inputLayout->addWidget(portLineEdit, 1, 1);

    mainLayout->addLayout(inputLayout);
    mainLayout->addWidget(clientProgressBar);
    mainLayout->addWidget(clientStatusLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(openButton);
    buttonLayout->addWidget(quitButton);

    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setWindowTitle(QStringLiteral("版本控制GIT檔案傳送"));

    // 信號與槽連接
    connect(openButton, &QPushButton::clicked, this, &TcpFileSender::openFile);
    connect(startButton, &QPushButton::clicked, this, &TcpFileSender::start);
    connect(quitButton, &QPushButton::clicked, this, &TcpFileSender::close);
    connect(&tcpClient, &QTcpSocket::connected, this, &TcpFileSender::startTransfer);
    connect(&tcpClient, &QTcpSocket::bytesWritten, this, &TcpFileSender::updateClientProgress);

    // 新增連線確認的功能
    connect(ipLineEdit, &QLineEdit::textChanged, this, &TcpFileSender::enableStartButton);
    connect(portLineEdit, &QLineEdit::textChanged, this, &TcpFileSender::enableStartButton);
}

void TcpFileSender::enableStartButton()
{
    startButton->setEnabled(!ipLineEdit->text().isEmpty() && !portLineEdit->text().isEmpty() && !fileName.isEmpty());
}

void TcpFileSender::openFile()
{
    fileName = QFileDialog::getOpenFileName(this, QStringLiteral("選擇檔案"));
    if (!fileName.isEmpty()) {
        clientStatusLabel->setText(QStringLiteral("檔案已選擇：%1").arg(fileName));
    }
}

void TcpFileSender::start()
{
    QString ip = ipLineEdit->text();
    QString port = portLineEdit->text();

    if (ip.isEmpty() || port.isEmpty()) {
        QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("IP 或 Port 不可為空"));
        return;
    }

    clientStatusLabel->setText(QStringLiteral("連接中..."));
    tcpClient.connectToHost(ip, port.toInt());
}

void TcpFileSender::startTransfer()
{
    localFile = new QFile(fileName);
    if (!localFile->open(QFile::ReadOnly)) {
        QMessageBox::warning(this, QStringLiteral("應用程式"),
                             QStringLiteral("無法讀取 %1:\n%2.").arg(fileName)
                                 .arg(localFile->errorString()));
        return;
    }

    totalBytes = localFile->size();
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut.setVersion(QDataStream::Qt_4_6);
    QString currentFile = fileName.right(fileName.size() - fileName.lastIndexOf("/") - 1);
    sendOut << qint64(0) << qint64(0) << currentFile;
    totalBytes += outBlock.size();

    sendOut.device()->seek(0);
    sendOut << totalBytes << qint64(outBlock.size() - sizeof(qint64) * 2);
    bytesToWrite = totalBytes - tcpClient.write(outBlock);
    clientStatusLabel->setText(QStringLiteral("已連接"));
    outBlock.resize(0);
}

void TcpFileSender::updateClientProgress(qint64 numBytes)
{
    bytesWritten += (int)numBytes;
    if (bytesToWrite > 0) {
        outBlock = localFile->read(qMin(bytesToWrite, loadSize));
        bytesToWrite -= (int)tcpClient.write(outBlock);
        outBlock.resize(0);
    } else {
        localFile->close();
    }

    clientProgressBar->setMaximum(totalBytes);
    clientProgressBar->setValue(bytesWritten);
    clientStatusLabel->setText(QStringLiteral("已傳送 %1 Bytes").arg(bytesWritten));
}

TcpFileSender::~TcpFileSender()
{
}
