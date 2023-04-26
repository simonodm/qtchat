#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "chatwindow.h"
#include "utils.h"

#include <settingswindow.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
    , configuration_(loadConfiguration())
{
    ui_->setupUi(this);

    QObject::connect(ui_->settingsButton, &QPushButton::clicked, this, &MainWindow::onSettingsButtonClicked);
    QObject::connect(ui_->connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    QObject::connect(ui_->listenCheckbox, &QCheckBox::stateChanged, this, &MainWindow::onListenCheckboxStateChanged);

    initializeSessionCreator();
}

MainWindow::~MainWindow()
{
    delete ui_;
}

void MainWindow::onChatRequestReceived(std::shared_ptr<ChatSession> session) {
    auto connectionDialog = createConnectionDialog();
    auto handshakeProcessor = std::make_unique<EncryptedSessionReceiverHandshakeProcessor>(configuration_.keys, configuration_.userInfo);
    onConnectionEstablished(session, connectionDialog);
    session->initialize(std::move(handshakeProcessor));
}

void MainWindow::onConnectionEstablished(std::shared_ptr<ChatSession> session, ConnectionDialog *connectionDialog) {
    if(connectionDialog != nullptr) {
        connectionDialog->setStatus(ConnectionProgress::EstablishingSession);
        QObject::connect(connectionDialog, &ConnectionDialog::cancelled, this, [this, session, connectionDialog] { onDisconnect(session, connectionDialog); });
    }

    QObject::connect(session.get(), &ChatSession::sessionInitialized, this, [this, session, connectionDialog] { onSessionEstablished(session, connectionDialog); });
    QObject::connect(session.get(), &ChatSession::sessionInitializationError, this, [this, session, connectionDialog] { onDisconnect(session, connectionDialog); });
    QObject::connect(this, &MainWindow::configurationChanged, session.get(), [this, session, connectionDialog] { onDisconnect(session, connectionDialog); });
}

void MainWindow::onSessionEstablished(std::shared_ptr<ChatSession> session, ConnectionDialog *connectionDialog) {
    if(connectionDialog != nullptr) {
        QObject::disconnect(connectionDialog, &ConnectionDialog::cancelled, this, nullptr); // need to disconnect this first to not trigger onDisconnect
        connectionDialog->close();
    }

    QObject::disconnect(session.get(), &ChatSession::sessionInitialized, this, nullptr);
    QObject::disconnect(session.get(), &ChatSession::sessionInitializationError, this, nullptr);
    QObject::disconnect(this, &MainWindow::configurationChanged, session.get(), nullptr);

    ChatWindow *chatWindow = new ChatWindow(session, this);
    chatWindow->setAttribute(Qt::WA_DeleteOnClose);
    chatWindow->show();
}

void MainWindow::onDisconnect(std::shared_ptr<ChatSession> session, ConnectionDialog *connectionDialog) {
    QObject::disconnect(session.get(), &ChatSession::connectionEstablished, this, nullptr);
    QObject::disconnect(session.get(), &ChatSession::sessionInitialized, this, nullptr);
    QObject::disconnect(session.get(), &ChatSession::sessionInitializationError, this, nullptr);
    QObject::disconnect(this, &MainWindow::configurationChanged, session.get(), nullptr);
    session->end();

    if(connectionDialog != nullptr) {
        connectionDialog->close();
    }
}

void MainWindow::handleConfigurationChange(Configuration &configuration) {
    configuration_ = configuration;
    configuration.saveToFile(Configuration::getDefaultConfigPath());
    emit configurationChanged();

    sessionCreator_->disallowConnections();
    sessionCreator_->setUserInfo(configuration.userInfo);
    sessionCreator_->setKeys(configuration.keys);

    if(ui_->listenCheckbox->checkState() == Qt::CheckState::Checked) {
        sessionCreator_->allowConnections(configuration.port);
    }
}

void MainWindow::onConnectButtonClicked()
{
    auto host = ui_->hostTextEdit->text().toStdString();
    int port;

    try {
        port = std::stoi(ui_->portTextEdit->text().toStdString());
    }
    catch (const std::invalid_argument &ex) {
        return;
    }

    auto session = sessionCreator_->tryConnect(host, port);

    auto connectionDialog = createConnectionDialog();
    QObject::connect(connectionDialog, &ConnectionDialog::cancelled, this, [this, session, connectionDialog] { onDisconnect(session, connectionDialog); });

    QObject::connect(session.get(), &ChatSession::connectionEstablished, this, [this, session, connectionDialog] {
        auto handshakeProcessor = std::make_unique<EncryptedSessionSenderHandshakeProcessor>(configuration_.keys, configuration_.userInfo);
        onConnectionEstablished(session, connectionDialog);
        session->initialize(std::move(handshakeProcessor));
    });
}

void MainWindow::onSettingsButtonClicked()
{
    auto settingsDialog = new SettingsWindow(configuration_, this);
    QObject::connect(settingsDialog, &SettingsWindow::configurationChanged, this, &MainWindow::handleConfigurationChange);
    settingsDialog->setAttribute(Qt::WA_DeleteOnClose);
    settingsDialog->show();
}

void MainWindow::onListenCheckboxStateChanged(int newState) {
    if(newState == Qt::CheckState::Checked) {
        sessionCreator_->allowConnections(configuration_.port);
    }
    else {
        sessionCreator_->disallowConnections();
    }
}

ConnectionDialog* MainWindow::createConnectionDialog() {
    auto connectionDialog = new ConnectionDialog(this);
    connectionDialog->setStatus(ConnectionProgress::Connecting);
    connectionDialog->setAttribute(Qt::WA_DeleteOnClose);
    connectionDialog->show();
    return connectionDialog;
}

void MainWindow::initializeSessionCreator() {
    sessionCreator_ = std::make_unique<ChatSessionCreator>(configuration_.userInfo, configuration_.keys);

    if(ui_->listenCheckbox->checkState() == Qt::CheckState::Checked) {
        sessionCreator_->allowConnections(configuration_.port);
    }
    QObject::connect(sessionCreator_.get(), &ChatSessionCreator::chatRequestReceived, this, &MainWindow::onChatRequestReceived);
}

Configuration MainWindow::loadConfiguration() {
    try {
        return Configuration::loadFromFile(Configuration::getDefaultConfigPath());
    }
    catch (std::runtime_error &ex) {
        auto config = Configuration::defaultConfiguration();
        config.saveToFile(Configuration::getDefaultConfigPath());
        return config;
    }
}

