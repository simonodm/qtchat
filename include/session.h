#ifndef SESSION_H
#define SESSION_H

#include "messaging.h"

#include <QObject>

/**
 * @brief An abstract class for converting std::string to Message and vice versa.
 */
class MessageConverter {
public:
    virtual std::shared_ptr<Message> convertToMessage(const std::string &message) const = 0;
    virtual std::string convertFromMessage(Message *message) = 0;
};

/**
 * @brief Standard message converter which assumes format:
 * [5B length] QC [1B type] [content]
 */
class StandardMessageConverter : public MessageConverter, protected MessageVisitor {
public:
    std::shared_ptr<Message> convertToMessage(const std::string &message) const override;
    std::string convertFromMessage(Message *message) override;

protected:
    void processMessage(KeyMessage *message) override;
    void processMessage(SessionEndMessage *message) override;
    void processMessage(UserInfoMessage *message) override;
    void processMessage(NewChatMessage *message) override;
    void processMessage(EditChatMessage *message) override;

private:
    struct MessageData {
        MessageData() {}
        MessageData(const std::string &content);
        char typeIdentifier;
        std::string messageContent;
    };

    MessageData current_;
};

/**
 * @brief Encrypted message converter which assumes format:
 * [5B length] QC [1B type] [encrypted content]
 */
class EncryptedMessageConverter : public StandardMessageConverter {
public:
    EncryptedMessageConverter(std::shared_ptr<EncryptingKey> encryptor, std::shared_ptr<DecryptingKey> decryptor) : encryptor_(encryptor), decryptor_(decryptor) {}
    std::shared_ptr<Message> convertToMessage(const std::string &message) const override;
    std::string convertFromMessage(Message *message) override;

private:
    std::shared_ptr<EncryptingKey> encryptor_ = nullptr;
    std::shared_ptr<DecryptingKey> decryptor_ = nullptr;
};

/**
 * @brief Abstract class for complete processing of session initialization handshake.
 */
class SessionHandshakeProcessor : public QObject {
    Q_OBJECT

public:
    virtual void startHandshake() = 0;
    virtual void end() = 0;

public slots:
    virtual void processMessage(const std::string &message) = 0;

signals:
    void messageReady(const std::string &message);

    void handshakeFinished(std::shared_ptr<MessageConverter> messageProcessor, UserInfo otherUserInfo);
    void handshakeError(const std::string &errorMessage = "");
};

/**
 * @brief Abstract class for encrypted handshake processing. Provides default handshake state management
 * and non-encryption related message visitor methods.
 */
class EncryptedSessionHandshakeProcessor : public SessionHandshakeProcessor, protected MessageVisitor {
    Q_OBJECT

public:
    EncryptedSessionHandshakeProcessor(const KeyCombination &keys, UserInfo userInfo);
    void end() override;

public slots:
    void processMessage(const std::string &message) override;

protected:
    void processMessage(SessionEndMessage *message) override;
    void processMessage(NewChatMessage *message) override;
    void processMessage(EditChatMessage *message) override;

    KeyCombination keys_;
    UserInfo userInfo_;

    std::shared_ptr<MessageConverter> messageConverter_;

    std::shared_ptr<EncryptingKey> encryptor_;
    std::shared_ptr<DecryptingKey> decryptor_;

    bool publicKeyReceived_ = false;
    bool finished_ = false;
};

/**
 * @brief Handles encrypted RSA/AES handshake initialization from the connection initiator's perspective.
 */
class EncryptedSessionSenderHandshakeProcessor : public EncryptedSessionHandshakeProcessor {
public:
    EncryptedSessionSenderHandshakeProcessor(const KeyCombination &keys, UserInfo userInfo) : EncryptedSessionHandshakeProcessor(keys, userInfo) {}
    void startHandshake() override;

protected:
    void processMessage(KeyMessage *message) override;
    void processMessage(UserInfoMessage *message) override;
};

/**
 * @brief Handles encrypted RSA/AES handshake initialization from the connection receiver's perspective.
 */
class EncryptedSessionReceiverHandshakeProcessor : public EncryptedSessionHandshakeProcessor {
public:
    EncryptedSessionReceiverHandshakeProcessor(const KeyCombination &keys, UserInfo userInfo);
    void startHandshake() override;

protected:
    void processMessage(KeyMessage *message) override;
    void processMessage(UserInfoMessage *message) override;
};

/**
 * @brief Represents the complete context of single chat session between two clients.
 */
class ChatSession : public QObject, public MessageVisitor {
    Q_OBJECT

public:
    ChatSession(std::shared_ptr<Connection> connection, UserInfo userInfo, const KeyCombination &keyCombination);
    UserInfo getOwnUserInfo() const { return ownUserInfo_; }
    UserInfo getOtherUserInfo() const { return otherUserInfo_; }
    void initialize(std::unique_ptr<SessionHandshakeProcessor> &&handshakeProcessor);

signals:
    void connectionEstablished();
    void sessionInitialized();
    void sessionInitializationError();
    void sessionEndedByOtherSide();

    void invalidMessageReceived(const std::string &errorMessage);

    void newChatMessageReceived(NewChatMessage *message);
    void editedChatMessageReceived(EditChatMessage *message);

public slots:
    void end();
    void sendMessage(std::shared_ptr<Message> message);

protected slots:
    void processMessage(KeyMessage *message) override;
    void processMessage(UserInfoMessage *message) override;
    void processMessage(SessionEndMessage *message) override;
    void processMessage(NewChatMessage *message) override;
    void processMessage(EditChatMessage *message) override;

private slots:
    void handleConnectionEstablished();
    void handleHandshakeFinish(std::shared_ptr<MessageConverter> messageProcessor, UserInfo otherUserInfo);
    void handleHandshakeError();
    void processReceivedMessage(const std::string &message);
    void handleDisconnect();

private:
    std::shared_ptr<Connection> connection_;
    std::vector<std::shared_ptr<AbstractChatMessage>> chatMessageHistory_;

    std::unique_ptr<SessionHandshakeProcessor> handshakeProcessor_;
    std::shared_ptr<MessageConverter> messageConverter_;

    KeyCombination keyCombination_;
    UserInfo ownUserInfo_;
    UserInfo otherUserInfo_;

    bool connected_ = false;
    bool initialized_ = false;
    bool ended_ = false;
};

/**
 * @brief Allows to receive and create chat session requests.
 */
class ChatSessionCreator : public QObject {
    Q_OBJECT

public:
    ChatSessionCreator(UserInfo userInfo, const KeyCombination &keyCombination);
    void allowConnections(int port);
    void disallowConnections();
    std::shared_ptr<ChatSession> tryConnect(std::string &host, int port);
    void setUserInfo(UserInfo userInfo);
    void setKeys(const KeyCombination &keyCombination);

signals:
    void chatRequestReceived(std::shared_ptr<ChatSession> session);

private slots:
    void handleConnectionReceived(std::shared_ptr<Connection> connection);

private:
    std::shared_ptr<ChatSession> createSession(std::shared_ptr<Connection> connection);

    std::unique_ptr<Server> connectionManager_;
    UserInfo userInfo_;
    KeyCombination encryptionKeys_;
};

#endif // SESSION_H
