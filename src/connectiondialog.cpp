#include "connectiondialog.h"
#include "ui_connectiondialog.h"

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);

    QObject::connect(ui->cancelButton, &QPushButton::clicked, this, &ConnectionDialog::onCancelButtonClicked);

    setWindowTitle("Connection status");
    setStatus(ConnectionProgress::Connecting);
}

void ConnectionDialog::setStatus(ConnectionProgress connectionProgress) {
    switch(connectionProgress) {
    case ConnectionProgress::Connecting:
        ui->statusLabel->setText("Connecting...");
        break;
    case ConnectionProgress::EstablishingSession:
        ui->statusLabel->setText("Establishing session...");
        break;
    case ConnectionProgress::Error:
        ui->statusLabel->setText("An error has occurred.");
        ui->cancelButton->setText("Close");
        break;
    case ConnectionProgress::Finished:
        ui->statusLabel->setText("Connected.");
        break;
    }
}

ConnectionDialog::~ConnectionDialog()
{
    emit cancelled();
    delete ui;
}

void ConnectionDialog::onCancelButtonClicked()
{
    emit cancelled();
}

