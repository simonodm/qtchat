#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include "messaging.h"

#include <QFrame>

namespace Ui {
class ChatMessage;
}

class ChatMessage : public QFrame
{
    Q_OBJECT

public:
    explicit ChatMessage(const std::string &username, NewChatMessage *message, bool isEditable = false, QWidget *parent = nullptr);
    explicit ChatMessage(NewChatMessage *message, bool isEditable = false, QWidget *parent = nullptr);
    std::string getId() { return id_; }
    void edit(const std::string &newContent);
    ~ChatMessage();

signals:
    void edited(std::shared_ptr<EditChatMessage> message);

private slots:
    void handleEdit(std::shared_ptr<EditChatMessage> message);
    void onEditButtonClicked();

private:
    void hideEdit();
    void hideUsername();

    Ui::ChatMessage *ui;
    std::string id_;
};

#endif // CHATMESSAGE_H
