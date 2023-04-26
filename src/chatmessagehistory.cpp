#include "chatmessagehistory.h"
#include "ui_chatmessagehistory.h"

#include <QScrollBar>

ChatMessageHistory::ChatMessageHistory(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ChatMessageHistory)
{
    ui->setupUi(this);
    auto verticalScroll = ui->scrollArea->verticalScrollBar();
    QObject::connect(verticalScroll, &QScrollBar::rangeChanged, this, &ChatMessageHistory::scrollToBottom);
}

void ChatMessageHistory::addMessage(const std::string &sender, NewChatMessage *message, bool isEditable) {
    ChatMessage *messageWidget;
    if(sender == lastMessageSender_) {
        messageWidget = new ChatMessage(message, isEditable, this);
    }
    else {
        messageWidget = new ChatMessage(sender, message, isEditable, this);
        lastMessageSender_ = sender;
    }

    if(isEditable) {
        QObject::connect(messageWidget, &ChatMessage::edited, this, &ChatMessageHistory::messageEdited);
    }

    addChatMessageWidget(messageWidget);
}

void ChatMessageHistory::handleMessageEdit(EditChatMessage *message) {
    try {
        auto messageWidget = messages_.at(message->getId());
        messageWidget->edit(message->getContent());
    }
    catch (const std::out_of_range&) {
        return;
    }
}

void ChatMessageHistory::addChatMessageWidget(ChatMessage *messageWidget) {
    messages_.insert(std::make_pair(messageWidget->getId(), messageWidget));

    // widget gets inserted second to last so it stays above the spacer
    auto count = ui->messagesLayout->count();
    ui->messagesLayout->insertWidget(count - 1, messageWidget, 0, Qt::AlignTop);
}

void ChatMessageHistory::scrollToBottom(int min, int max) {
    ui->scrollArea->verticalScrollBar()->setValue(max);
}

ChatMessageHistory::~ChatMessageHistory()
{
    delete ui;
}
