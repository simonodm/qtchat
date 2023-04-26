#ifndef CHATMESSAGEHISTORY_H
#define CHATMESSAGEHISTORY_H

#include "messaging.h"
#include <chatmessage.h>

#include <QFrame>

namespace Ui {
class ChatMessageHistory;
}

class ChatMessageHistory : public QFrame
{
    Q_OBJECT

public:
    explicit ChatMessageHistory(QWidget *parent = nullptr);
    ~ChatMessageHistory();

signals:
    void messageEdited(std::shared_ptr<EditChatMessage> message);

public slots:
    void addMessage(const std::string &sender, NewChatMessage* message, bool isEditable = false);
    void handleMessageEdit(EditChatMessage* message);

private slots:
    void scrollToBottom(int min, int max);

private:
    void addChatMessageWidget(ChatMessage *messageWidget);

    Ui::ChatMessageHistory *ui;
    std::unordered_map<std::string, ChatMessage*> messages_;

    std::string lastMessageSender_;
};

#endif // CHATMESSAGEHISTORY_H
