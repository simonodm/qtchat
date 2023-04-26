#ifndef CHATMESSAGEEDITDIALOG_H
#define CHATMESSAGEEDITDIALOG_H

#include "messaging.h"

#include <QDialog>

namespace Ui {
class ChatMessageEditDialog;
}

class ChatMessageEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatMessageEditDialog(const std::string &id, const std::string &content, QWidget *parent = nullptr);
    ~ChatMessageEditDialog();

signals:
    void edited(std::shared_ptr<EditChatMessage> message);

private slots:
    void onSaveButtonClicked();

private:
    Ui::ChatMessageEditDialog *ui;
    std::string id_;
};

#endif // CHATMESSAGEEDITDIALOG_H
