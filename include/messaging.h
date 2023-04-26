#ifndef MESSAGING_H
#define MESSAGING_H

#include "configuration.h"
#include "encryption.h"
#include "network.h"

class MessageVisitor;

/**
 * @brief An abstract class for data messages which can be exchanged between clients.
 */
class Message {
public:
    virtual void process(MessageVisitor *handler) = 0;
};

/**
 * @brief Represents a message containing a cryptographic key.
 */
class KeyMessage : public Message {
public:
    KeyMessage(std::string encodedKey) : key_(encodedKey) {}
    std::string getEncodedKey() const { return key_; }
    void process(MessageVisitor *handler) override;
private:
    std::string key_;
};

/**
 * @brief Represents a message signalizing that the sender ended the session.
 */
class SessionEndMessage : public Message {
public:
    void process(MessageVisitor *handler) override;
};

/**
 * @brief Represents a message containing user information about the sender.
 */
class UserInfoMessage : public Message {
public:
    UserInfoMessage(UserInfo &userInfo) : userInfo_(userInfo) {}
    UserInfo getUserInfo() { return userInfo_; }
    void process(MessageVisitor *handler) override;
private:
    UserInfo userInfo_;
};

/**
 * @brief An abstract class for a uniquly-identifiable chat message.
 */
class AbstractChatMessage : public Message {
public:
    virtual std::string getId() const { return id_; }
    virtual std::string getContent() const { return message_; }

protected:
    AbstractChatMessage(const std::string &id, const std::string &content) : id_(id), message_(content) {}
    AbstractChatMessage(const std::string &content) : id_(generateId()), message_(content) {}
    std::string generateId();

protected:
    std::string id_;
    std::string message_;
};

/**
 * @brief Represents a new chat message.
 */
class NewChatMessage : public AbstractChatMessage {
public:
    NewChatMessage(const std::string &id, const std::string &message) : AbstractChatMessage(id, message) {}
    NewChatMessage(const std::string &message) : AbstractChatMessage(message) {}
    void process(MessageVisitor *handler) override;
};

/**
 * @brief Represents a message edit.
 */
class EditChatMessage : public AbstractChatMessage {
public:
    EditChatMessage(const std::string &id, const std::string &message) : AbstractChatMessage(id, message) {}
    void process(MessageVisitor *handler) override;
};

/**
 * @brief An abstract message visitor.
 */
class MessageVisitor {
public:
    virtual void processMessage(KeyMessage *message) = 0;
    virtual void processMessage(SessionEndMessage *message) = 0;
    virtual void processMessage(UserInfoMessage *message) = 0;
    virtual void processMessage(NewChatMessage *message) = 0;
    virtual void processMessage(EditChatMessage *message) = 0;
};



#endif // MESSAGING_H
