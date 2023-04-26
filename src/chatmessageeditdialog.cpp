#include "chatmessageeditdialog.h"
#include "ui_chatmessageeditdialog.h"

ChatMessageEditDialog::ChatMessageEditDialog(const std::string &id, const std::string &content, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChatMessageEditDialog)
{
    ui->setupUi(this);
    id_ = id;
    ui->messageEdit->setText(QString::fromStdString(content));

    QObject::connect(ui->saveButton, &QPushButton::clicked, this, &ChatMessageEditDialog::onSaveButtonClicked);
}

ChatMessageEditDialog::~ChatMessageEditDialog()
{
    delete ui;
}

void ChatMessageEditDialog::onSaveButtonClicked()
{
    auto message = std::make_shared<EditChatMessage>(id_, ui->messageEdit->text().toStdString());
    emit edited(message);
    close();
}

