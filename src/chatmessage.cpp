#include "chatmessage.h"
#include "chatmessageeditdialog.h"
#include "ui_chatmessage.h"

ChatMessage::ChatMessage(const std::string &username, NewChatMessage *message, bool isEditable, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ChatMessage)
{
    ui->setupUi(this);

    QObject::connect(ui->editButton, &QPushButton::clicked, this, &ChatMessage::onEditButtonClicked);

    ui->username->setText(QString::fromStdString(username));
    ui->message->setText(QString::fromStdString(message->getContent()));
    if(!isEditable) {
        hideEdit();
    }

    id_ = message->getId();
}

ChatMessage::ChatMessage(NewChatMessage *message, bool isEditable, QWidget *parent) :
    ChatMessage("", message, isEditable, parent)
{
    hideUsername();
}

void ChatMessage::edit(const std::string &newContent) {
    ui->message->setText(QString::fromStdString(newContent));
}

void ChatMessage::handleEdit(std::shared_ptr<EditChatMessage> message) {
    edit(message->getContent());
    emit edited(message);
}

void ChatMessage::hideEdit() {
    ui->gridLayout->removeWidget(ui->editButton);
    ui->editButton->hide();
}

void ChatMessage::hideUsername() {
    ui->gridLayout->removeWidget(ui->username);
}

ChatMessage::~ChatMessage()
{
    delete ui;
}

void ChatMessage::onEditButtonClicked()
{
    auto editDialog = new ChatMessageEditDialog(id_, ui->message->text().toStdString(), this);
    QObject::connect(editDialog, &ChatMessageEditDialog::edited, this, &ChatMessage::handleEdit);
    editDialog->setAttribute(Qt::WA_DeleteOnClose);
    editDialog->show();
}

