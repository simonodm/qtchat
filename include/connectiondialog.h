#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ConnectionDialog;
}

enum ConnectionProgress {
    Connecting,
    EstablishingSession,
    Error,
    Finished
};

class ConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnectionDialog(QWidget *parent = nullptr);
    void setStatus(ConnectionProgress connectionProgress);
    ~ConnectionDialog();

signals:
    void cancelled();

private slots:
    void onCancelButtonClicked();

private:
    Ui::ConnectionDialog *ui;
};

#endif // CONNECTIONDIALOG_H
