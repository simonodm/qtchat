#include "settingswindow.h"
#include "ui_settingswindow.h"

#include <QFileDialog>
#include <QIntValidator>

SettingsWindow::SettingsWindow(Configuration config, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsWindow),
    config_(config)
{
    ui->setupUi(this);

    QObject::connect(ui->openPublicKeyFileButton, &QPushButton::clicked, this, &SettingsWindow::onOpenPublicKeyFileButtonClicked);
    QObject::connect(ui->openPrivateKeyFileButton, &QPushButton::clicked, this, &SettingsWindow::onOpenPrivateKeyFileButtonClicked);
    QObject::connect(ui->saveButton, &QPushButton::clicked, this, &SettingsWindow::onSaveButtonClicked);

    ui->publicKeyPathEdit->setText(QString::fromStdString(config.publicKeyFile));
    ui->privateKeyPathEdit->setText(QString::fromStdString(config.privateKeyFile));
    ui->usernameEdit->setText(QString::fromStdString(config.userInfo.getUsername()));
    ui->portEdit->setValidator(new QIntValidator);
    ui->portEdit->setText(QString::fromStdString(std::to_string(config.port)));
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::onOpenPublicKeyFileButtonClicked()
{
    auto fileDialog = new QFileDialog(this);
    fileDialog->setFileMode(QFileDialog::FileMode::AnyFile);
    fileDialog->setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    fileDialog->setOption(QFileDialog::Option::DontConfirmOverwrite);
    fileDialog->setNameFilter(tr("PEM keys (*.pem)"));
    QObject::connect(fileDialog, &QFileDialog::fileSelected, this, &SettingsWindow::onPublicKeyFileSelected);
    fileDialog->show();
}


void SettingsWindow::onOpenPrivateKeyFileButtonClicked()
{
    auto fileDialog = new QFileDialog(this);
    fileDialog->setFileMode(QFileDialog::FileMode::AnyFile);
    fileDialog->setAcceptMode(QFileDialog::AcceptMode::AcceptSave);
    fileDialog->setOption(QFileDialog::Option::DontConfirmOverwrite);
    fileDialog->setNameFilter(tr("PEM keys (*.pem)"));
    QObject::connect(fileDialog, &QFileDialog::fileSelected, this, &SettingsWindow::onPrivateKeyFileSelected);
    fileDialog->show();
}


void SettingsWindow::onSaveButtonClicked()
{
    auto publicKeyFile = ui->publicKeyPathEdit->text().toStdString();
    auto privateKeyFile = ui->privateKeyPathEdit->text().toStdString();
    auto username = ui->usernameEdit->text().toStdString();
    auto port = std::stoi(ui->portEdit->text().toStdString());

    auto config = Configuration(publicKeyFile, privateKeyFile, username, port);
    emit configurationChanged(config);

    close();
}

void SettingsWindow::onPublicKeyFileSelected(const QString &file) {
    ui->publicKeyPathEdit->setText(file);
}

void SettingsWindow::onPrivateKeyFileSelected(const QString &file) {
    ui->privateKeyPathEdit->setText(file);
}

