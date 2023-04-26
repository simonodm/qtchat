#include "messaging.h"

std::string AbstractChatMessage::generateId() {
    std::string result;

    for(auto i = 0; i < 8; ++i) {
        result += ('a' + std::rand()%26);
    }

    return result;
}

void KeyMessage::process(MessageVisitor *handler) {
    handler->processMessage(this);
}

void SessionEndMessage::process(MessageVisitor *handler) {
    handler->processMessage(this);
}

void UserInfoMessage::process(MessageVisitor *handler) {
    handler->processMessage(this);
}

void NewChatMessage::process(MessageVisitor *handler) {
    handler->processMessage(this);
}

void EditChatMessage::process(MessageVisitor *handler) {
    handler->processMessage(this);
}
