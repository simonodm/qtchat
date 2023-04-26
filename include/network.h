#ifndef NETWORK_H
#define NETWORK_H

#include <QTcpServer>
#include <QTcpSocket>

/**
 * @brief An abstract class representing a single socket connection.
 */
class Connection : public QObject {
    Q_OBJECT

public:
    virtual bool isConnected() = 0;
    virtual void close() = 0;

public slots:
    virtual void send(const std::string &message) const = 0;

signals:
    void connected();
    void disconnected();
    void messageReceived(const std::string &content);
};

/**
 * @brief Represents a TCP socket connection.
 */
class TcpConnection : public Connection {
    Q_OBJECT

public:
    TcpConnection(QTcpSocket *socket);
    void close() override;
    bool isConnected() override;

public slots:
    void send(const std::string &data) const override;

private slots:
    void handleSocketReadyRead();

private:
    void tryParseCurrentMessage();
    unsigned int getMessageSize(const std::string &message) const;

    QTcpSocket *socket_;
    std::string messageBuffer_;
};

/**
 * @brief An abstract class representing a server.
 */
class Server : public QObject {
    Q_OBJECT

public:
    virtual void listen(uint port) = 0;
    virtual void stopListening() = 0;
    virtual std::shared_ptr<Connection> connect(const std::string &host, uint port) = 0;

signals:
    void connectionReceived(std::shared_ptr<Connection> connection);
};

/**
 * @brief Represents a TCP server.
 */
class TcpServer : public Server {
    Q_OBJECT

public:
    TcpServer();
    void listen(uint port) override;
    void stopListening() override;
    std::shared_ptr<Connection> connect(const std::string &host, uint port) override;
    ~TcpServer();

private slots:
    void processNewConnection();

private:
    QTcpServer *server_;
};

#endif // NETWORK_H
