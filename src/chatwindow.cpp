#include "chatwindow.h"
#include "ui_chatwindow.h"

ChatWindow::ChatWindow(std::shared_ptr<ChatSession> chatSession, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatWindow),
    chatSession_(chatSession)
{
    ui->setupUi(this);

    QObject::connect(ui->sendMessageButton, &QPushButton::clicked, this, &ChatWindow::onSendMessageButtonClicked);

    QObject::connect(chatSession.get(), &ChatSession::newChatMessageReceived, this, &ChatWindow::onNewMessageReceived);
    QObject::connect(chatSession.get(), &ChatSession::editedChatMessageReceived, this, &ChatWindow::onMessageEditReceived);
    QObject::connect(chatSession.get(), &ChatSession::sessionEndedByOtherSide, this, &ChatWindow::handleSessionEnded);
    QObject::connect(ui->chatMessageHistory, &ChatMessageHistory::messageEdited, this, &ChatWindow::handleMessageEdited);
}

void ChatWindow::onNewMessageReceived(NewChatMessage *message) {
    ui->chatMessageHistory->addMessage(chatSession_->getOtherUserInfo().getUsername(), message);
}

void ChatWindow::onMessageEditReceived(EditChatMessage *message) {
    ui->chatMessageHistory->handleMessageEdit(message);
}

void ChatWindow::handleMessageEdited(std::shared_ptr<EditChatMessage> message) {
    chatSession_->sendMessage(message);
}

void ChatWindow::handleSessionEnded() {
    QObject::disconnect(chatSession_.get(), nullptr, this, nullptr);
    QObject::disconnect(ui->chatMessageHistory, nullptr, this, nullptr);

    ui->chatMessageTextEdit->setDisabled(true);
    ui->chatMessageHistory->setDisabled(true);
    ui->sendMessageButton->setDisabled(true);

    setWindowTitle(this->windowTitle() + " - Disconnected");
}

ChatWindow::~ChatWindow()
{
    chatSession_->end();
    delete ui;
}

void ChatWindow::onSendMessageButtonClicked()
{
    auto content = ui->chatMessageTextEdit->text();
    auto message = std::make_shared<NewChatMessage>(content.toStdString());

    ui->chatMessageHistory->addMessage(chatSession_->getOwnUserInfo().getUsername(), message.get(), true);
    ui->chatMessageTextEdit->setText("");

    chatSession_->sendMessage(message);
}

