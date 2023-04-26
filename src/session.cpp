#include <iomanip>
#include <sstream>
#include "session.h"
#include "utils.h"

const std::string INVALID_MESSAGE_ERROR = "Invalid message received.";
const std::string UNKNOWN_MESSAGE_TYPE_ERROR = "Unknown message type received.";
const std::string HANDSHAKE_ALREADY_FINISHED_ERROR = "Handshake processing was already finished or terminated.";
const std::string DUPLICATE_KEY_ERROR = "Duplicate key received.";
const std::string HANDSHAKE_TERMINATED_ERROR = "Session terminated by the other side.";
const std::string DATA_RECEIVED_BEFORE_KEY = "Data was received before encryption was established.";

std::shared_ptr<Message> StandardMessageConverter::convertToMessage(const std::string &message) const {
    auto messageData = MessageData(message);

    switch(messageData.typeIdentifier) {
    case 'K': {
        return std::make_shared<KeyMessage>(messageData.messageContent);
    }
    case 'U': {
        auto userInfo = UserInfo(messageData.messageContent);
        return std::make_shared<UserInfoMessage>(userInfo);
    }
    case 'S': {
        return std::make_shared<SessionEndMessage>();
    }
    case 'N': {
        if(messageData.messageContent.length() < 9) {
            throw std::runtime_error(INVALID_MESSAGE_ERROR);
        }
        auto id = messageData.messageContent.substr(0, 8);
        auto messageContent = messageData.messageContent.substr(8);
        return std::make_shared<NewChatMessage>(id, messageContent);
    }
    case 'E': {
        if(messageData.messageContent.length() < 9) {
            throw std::runtime_error(INVALID_MESSAGE_ERROR);
        }
        auto id = messageData.messageContent.substr(0, 8);
        auto messageContent = messageData.messageContent.substr(8);
        return std::make_shared<EditChatMessage>(id, messageContent);
    }
    default:
        throw std::runtime_error(UNKNOWN_MESSAGE_TYPE_ERROR);
    }
}

StandardMessageConverter::MessageData::MessageData(const std::string &content) {
    if(content.length() < 8 || content[5] != 'Q' || content[6] != 'C') {
        throw std::runtime_error(INVALID_MESSAGE_ERROR);
    }

    typeIdentifier = content[7];
    auto length = std::stoi(content.substr(0, 5), 0, 16);
    if(content.length() < length) {
        throw std::runtime_error(INVALID_MESSAGE_ERROR);
    }

    messageContent = content.substr(8, length);
}

std::string StandardMessageConverter::convertFromMessage(Message *message) {
    message->process(this);

    std::stringstream ss;
    ss << Utils::convertToHex(current_.messageContent.length() + 8, 5) << "QC" << current_.typeIdentifier << current_.messageContent;
    return ss.str();
}

void StandardMessageConverter::processMessage(KeyMessage *message) {
    current_.typeIdentifier = 'K';
    current_.messageContent = message->getEncodedKey();
}

void StandardMessageConverter::processMessage(SessionEndMessage *message) {
    current_.typeIdentifier = 'S';
    current_.messageContent = "";
}

void StandardMessageConverter::processMessage(UserInfoMessage *message) {
    current_.typeIdentifier = 'U';
    current_.messageContent = message->getUserInfo().getUsername();
}

void StandardMessageConverter::processMessage(NewChatMessage *message) {
    current_.typeIdentifier = 'N';
    current_.messageContent = message->getId() + message->getContent();
}

void StandardMessageConverter::processMessage(EditChatMessage *message) {
    current_.typeIdentifier = 'E';
    current_.messageContent = message->getId() + message->getContent();
}

std::shared_ptr<Message> EncryptedMessageConverter::convertToMessage(const std::string &message) const {
    if(decryptor_ == nullptr) {
        return StandardMessageConverter::convertToMessage(message);
    }

    auto decryptedMessageContent = decryptor_->decrypt(message.substr(8));
    std::stringstream ss;
    ss << Utils::convertToHex(decryptedMessageContent.length() + 8, 5) << "QC" << message[7] << decryptedMessageContent;
    return StandardMessageConverter::convertToMessage(ss.str());
}

std::string EncryptedMessageConverter::convertFromMessage(Message *message) {
    auto encodedMessage = StandardMessageConverter::convertFromMessage(message);
    if(encryptor_ == nullptr) {
        return encodedMessage;
    }

    auto encryptedContent = encryptor_->encrypt(encodedMessage.substr(8));
    auto type = encodedMessage[7];
    std::stringstream ss;
    ss << Utils::convertToHex(encryptedContent.length() + 8, 5) << "QC" << type << encryptedContent;
    return ss.str();
}

EncryptedSessionHandshakeProcessor::EncryptedSessionHandshakeProcessor(const KeyCombination &keys, UserInfo userInfo)
    : keys_(keys),
      decryptor_(keys.getPrivateKey()),
      userInfo_(userInfo)
{
    messageConverter_ = std::make_shared<StandardMessageConverter>();
}

void EncryptedSessionHandshakeProcessor::end() {
    auto message = std::make_shared<SessionEndMessage>();
    auto encodedMessage = messageConverter_->convertFromMessage(message.get());
    emit messageReady(encodedMessage);
}

void EncryptedSessionHandshakeProcessor::processMessage(const std::string &message) {
    if(finished_) {
        emit handshakeError(HANDSHAKE_ALREADY_FINISHED_ERROR);
    }

    try {
        auto decodedMessage = messageConverter_->convertToMessage(message);
        decodedMessage->process(this);
    }
    catch(std::runtime_error &error) {
        emit handshakeError(error.what());
    }
}

void EncryptedSessionHandshakeProcessor::processMessage(SessionEndMessage *message) {    
    finished_ = true;
    emit handshakeError(HANDSHAKE_TERMINATED_ERROR);
}

void EncryptedSessionHandshakeProcessor::processMessage(NewChatMessage *message) {
    emit handshakeError(INVALID_MESSAGE_ERROR);
}

void EncryptedSessionHandshakeProcessor::processMessage(EditChatMessage *message) {
    emit handshakeError(INVALID_MESSAGE_ERROR);
}

void EncryptedSessionSenderHandshakeProcessor::startHandshake() {
    // do nothing, wait for public key from connection receiver
}

void EncryptedSessionSenderHandshakeProcessor::processMessage(KeyMessage *message) {
    if(publicKeyReceived_) {
        finished_ = true;
        emit handshakeError(DUPLICATE_KEY_ERROR);
    }

    std::shared_ptr<RSAPublicKey> rsaKey(RSAPublicKey::decodeFromPEM(message->getEncodedKey()));
    auto aesKey = std::make_shared<AESKey>();
    auto tempMessageConverter = std::make_shared<EncryptedMessageConverter>(rsaKey, aesKey);
    publicKeyReceived_ = true;

    auto messageToSend = std::make_shared<KeyMessage>(aesKey->encode());
    auto encryptedMessage = tempMessageConverter->convertFromMessage(messageToSend.get());

    messageConverter_ = std::make_shared<EncryptedMessageConverter>(aesKey, aesKey);
    emit messageReady(encryptedMessage);
}

void EncryptedSessionSenderHandshakeProcessor::processMessage(UserInfoMessage *message) {
    auto ownUserInfoMessage = std::make_shared<UserInfoMessage>(userInfo_);
    auto encryptedMessage = messageConverter_->convertFromMessage(ownUserInfoMessage.get());
    emit messageReady(encryptedMessage);
    emit handshakeFinished(messageConverter_, message->getUserInfo());
}

EncryptedSessionReceiverHandshakeProcessor::EncryptedSessionReceiverHandshakeProcessor(const KeyCombination &keys, UserInfo userInfo)
    : EncryptedSessionHandshakeProcessor(keys, userInfo)
{
    messageConverter_ = std::make_shared<EncryptedMessageConverter>(nullptr, keys_.getPrivateKey());
}

void EncryptedSessionReceiverHandshakeProcessor::startHandshake() {
    auto message = std::make_shared<KeyMessage>(keys_.getPublicKey()->encode());
    auto encodedMessage = messageConverter_->convertFromMessage(message.get());
    emit messageReady(encodedMessage);
}

void EncryptedSessionReceiverHandshakeProcessor::processMessage(KeyMessage *message) {
    // AES key expected
    auto aesKey = std::make_shared<AESKey>(message->getEncodedKey());
    messageConverter_ = std::make_shared<EncryptedMessageConverter>(aesKey, aesKey);
    auto messageToSend = std::make_shared<UserInfoMessage>(userInfo_);
    auto encryptedMessage = messageConverter_->convertFromMessage(messageToSend.get());
    publicKeyReceived_ = true;

    emit messageReady(encryptedMessage);
}

void EncryptedSessionReceiverHandshakeProcessor::processMessage(UserInfoMessage *message) {
    if(!publicKeyReceived_) {
        finished_ = true;
        emit handshakeError(DATA_RECEIVED_BEFORE_KEY);
    }

    emit handshakeFinished(messageConverter_, message->getUserInfo());
}

ChatSession::ChatSession(std::shared_ptr<Connection> connection, UserInfo userInfo, const KeyCombination &keyCombination) :
    connection_(connection),
    ownUserInfo_(userInfo),
    keyCombination_(keyCombination)
{
    if(connection->isConnected()) {
        connected_ = true;
        emit connectionEstablished();
    }
    else {
        QObject::connect(connection.get(), &Connection::connected, this, &ChatSession::handleConnectionEstablished);
    }
}

void ChatSession::initialize(std::unique_ptr<SessionHandshakeProcessor> &&handshakeProcessor) {
    if(!connected_) {
        throw std::runtime_error("Connection has not been established yet.");
    }

    QObject::connect(connection_.get(), &Connection::disconnected, this, &ChatSession::handleDisconnect);

    handshakeProcessor_ = std::move(handshakeProcessor);
    QObject::connect(connection_.get(), &Connection::messageReceived, handshakeProcessor_.get(), &SessionHandshakeProcessor::processMessage);
    QObject::connect(handshakeProcessor_.get(), &SessionHandshakeProcessor::messageReady, connection_.get(), &Connection::send);
    QObject::connect(handshakeProcessor_.get(), &SessionHandshakeProcessor::handshakeFinished, this, &ChatSession::handleHandshakeFinish);
    QObject::connect(handshakeProcessor_.get(), &SessionHandshakeProcessor::handshakeError, this, &ChatSession::handleHandshakeError);
    handshakeProcessor_->startHandshake();
}

void ChatSession::end() {
    if(ended_) {
        return;
    }

    ended_ = true;

    if(initialized_) {
        auto sessionEndMessage = std::make_shared<SessionEndMessage>();
        sendMessage(sessionEndMessage);
    }
    else if(handshakeProcessor_ != nullptr) {
        handshakeProcessor_->end();
    }

    QObject::disconnect(handshakeProcessor_.get(), nullptr, this, nullptr);

    connection_->close();
}

void ChatSession::sendMessage(std::shared_ptr<Message> message) {
    auto encoded = messageConverter_->convertFromMessage(message.get());
    connection_->send(encoded);
}

void ChatSession::processMessage(KeyMessage *message) {
    emit invalidMessageReceived(DUPLICATE_KEY_ERROR);
}

void ChatSession::processMessage(UserInfoMessage *message) {
    otherUserInfo_ = message->getUserInfo();
}

void ChatSession::processMessage(SessionEndMessage *message) {
    ended_ = true;
    emit sessionEndedByOtherSide();
}

void ChatSession::processMessage(NewChatMessage *message) {
    emit newChatMessageReceived(message);
}

void ChatSession::processMessage(EditChatMessage *message) {
    emit editedChatMessageReceived(message);
}

void ChatSession::handleConnectionEstablished() {
    connected_ = true;
    QObject::disconnect(connection_.get(), &Connection::connected, this, nullptr);
    emit connectionEstablished();
}

void ChatSession::handleHandshakeFinish(std::shared_ptr<MessageConverter> messagePreprocessor, UserInfo otherUserInfo) {
    initialized_ = true;
    messageConverter_ = messagePreprocessor;
    otherUserInfo_ = otherUserInfo;
    QObject::disconnect(connection_.get(), &Connection::messageReceived, handshakeProcessor_.get(), &SessionHandshakeProcessor::processMessage);
    QObject::disconnect(handshakeProcessor_.get(), nullptr, this, nullptr);
    QObject::connect(connection_.get(), &Connection::messageReceived, this, &ChatSession::processReceivedMessage);
    emit sessionInitialized();
}

void ChatSession::handleHandshakeError() {
    QObject::disconnect(connection_.get(), &Connection::messageReceived, handshakeProcessor_.get(), &SessionHandshakeProcessor::processMessage);
    QObject::disconnect(handshakeProcessor_.get(), nullptr, this, nullptr);
    emit sessionInitializationError();
}

void ChatSession::handleDisconnect() {
    QObject::disconnect(connection_.get(), nullptr, this, nullptr);
    QObject::disconnect(handshakeProcessor_.get(), nullptr, this, nullptr);
    if(!initialized_) {
        emit sessionInitializationError();
    }
    else {
        emit sessionEndedByOtherSide();
    }
}

void ChatSession::processReceivedMessage(const std::string &message) {
    try {
        auto convertedMessage = messageConverter_->convertToMessage(message);
        convertedMessage->process(this);
    }
    catch (std::runtime_error &error) {
        emit invalidMessageReceived(error.what());
    }
}

ChatSessionCreator::ChatSessionCreator(UserInfo userInfo, const KeyCombination &keyCombination) :
    userInfo_(userInfo),
    encryptionKeys_(keyCombination)
{
    connectionManager_ = std::make_unique<TcpServer>();
}

void ChatSessionCreator::allowConnections(int port) {
    connectionManager_->listen(port);
    QObject::connect(connectionManager_.get(), &Server::connectionReceived, this, &ChatSessionCreator::handleConnectionReceived);
}

void ChatSessionCreator::disallowConnections() {
    connectionManager_->stopListening();
    QObject::disconnect(connectionManager_.get(), &Server::connectionReceived, this, &ChatSessionCreator::handleConnectionReceived);
}

std::shared_ptr<ChatSession> ChatSessionCreator::tryConnect(std::string &host, int port) {
    auto connection = connectionManager_->connect(host, port);
    return createSession(connection);
}

void ChatSessionCreator::setUserInfo(UserInfo userInfo) {
    userInfo_ = userInfo;
}

void ChatSessionCreator::setKeys(const KeyCombination &keyCombination) {
    encryptionKeys_ = keyCombination;
}

void ChatSessionCreator::handleConnectionReceived(std::shared_ptr<Connection> connection) {
    auto session = createSession(connection);
    emit chatRequestReceived(session);
}

std::shared_ptr<ChatSession> ChatSessionCreator::createSession(std::shared_ptr<Connection> connection) {
    auto session = std::make_shared<ChatSession>(connection, userInfo_, encryptionKeys_);
    return session;
}
