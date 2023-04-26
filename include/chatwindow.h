#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include "session.h"
#include "chatmessagehistory.h"

#include <QDialog>

namespace Ui {
class ChatWindow;
}

class ChatWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ChatWindow(std::shared_ptr<ChatSession> chatSession, QWidget *parent = nullptr);
    ~ChatWindow();

public slots:
    void onNewMessageReceived(NewChatMessage *message);
    void onMessageEditReceived(EditChatMessage *message);

signals:
    void messageSent(std::shared_ptr<NewChatMessage> message);

private slots:
    void handleMessageEdited(std::shared_ptr<EditChatMessage> message);
    void handleSessionEnded();
    void onSendMessageButtonClicked();

private:
    Ui::ChatWindow *ui;
    std::shared_ptr<ChatSession> chatSession_;
};

#endif // CHATWINDOW_H
