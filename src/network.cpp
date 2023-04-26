#include "network.h"
#include <sstream>

#include <QByteArray>

TcpConnection::TcpConnection(QTcpSocket *socket) : socket_(socket) {
    socket->setParent(this);
    QObject::connect(socket_, &QTcpSocket::connected, this, &TcpConnection::connected);
    QObject::connect(socket_, &QTcpSocket::disconnected, this, &TcpConnection::disconnected);
    QObject::connect(socket_, &QTcpSocket::errorOccurred, this, &TcpConnection::disconnected);
    QObject::connect(socket_, &QTcpSocket::readyRead, this, &TcpConnection::handleSocketReadyRead);
}

void TcpConnection::send(const std::string &data) const {
    if(socket_ == nullptr) {
        throw std::runtime_error("Invalid connection.");
    }

    socket_->write(QByteArray(data.c_str(), data.length()));
}

void TcpConnection::close() {
    if(socket_->isOpen()) {
        socket_->close();
    }
}

bool TcpConnection::isConnected() {
    return socket_->state() == QTcpSocket::SocketState::ConnectedState;
}

void TcpConnection::handleSocketReadyRead() {
    auto data = socket_->readAll();
    messageBuffer_ += data.toStdString();

    try {
        tryParseCurrentMessage();
    }
    catch (...) {
        close();
        emit disconnected();
    }
}

void TcpConnection::tryParseCurrentMessage() {
    // Loop until there are no complete messages left in buffer
    while(true) {
        if(messageBuffer_.length() < 5) { // first 5 bytes are message length
            return;
        }

        auto messageLength = getMessageSize(messageBuffer_);
        if(messageBuffer_.length() < messageLength) {
            return;
        }

        auto resultMessage = messageBuffer_.substr(0, messageLength);
        messageBuffer_ = messageBuffer_.substr(messageLength);
        emit messageReceived(resultMessage);
    }
}

unsigned int TcpConnection::getMessageSize(const std::string &message) const {
    auto hexLength = message.substr(0, 5);
    unsigned long length = std::stoul(hexLength, 0, 16);

    // no range checks required while length can be represented in 5 hex characters
    return length;
}

TcpServer::TcpServer() {
    server_ = new QTcpServer();
}

TcpServer::~TcpServer() {
    server_->close();
    delete server_;
}

void TcpServer::listen(uint port) {
    stopListening();

    server_->listen(QHostAddress::Any, port);
    QObject::connect(server_, &QTcpServer::newConnection, this, &TcpServer::processNewConnection);
}

void TcpServer::stopListening() {
    if(server_->isListening()) {
        server_->close();
        QObject::disconnect(server_);
    }
}

void TcpServer::processNewConnection() {
    auto socketPtr = server_->nextPendingConnection();
    if(socketPtr == nullptr) {
        return;
    }

    auto connection = std::make_shared<TcpConnection>(socketPtr);
    emit connectionReceived(connection);
}

std::shared_ptr<Connection> TcpServer::connect(const std::string &host, uint port) {
    auto socket = new QTcpSocket();
    socket->connectToHost(QString::fromUtf8(host), port);

    auto connection = std::make_shared<TcpConnection>(socket);
    return connection;
}
