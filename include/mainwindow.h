#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "configuration.h"
#include "connectiondialog.h"
#include "messaging.h"
#include "network.h"
#include "session.h"

#include <QMainWindow>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();    

signals:
    void configurationChanged();

private slots:
    void onChatRequestReceived(std::shared_ptr<ChatSession> session);
    void onConnectionEstablished(std::shared_ptr<ChatSession> session, ConnectionDialog *connectionDialog = nullptr);
    void onSessionEstablished(std::shared_ptr<ChatSession> session, ConnectionDialog *connectionDialog = nullptr);
    void onDisconnect(std::shared_ptr<ChatSession> session, ConnectionDialog *connectionDialog = nullptr);
    void handleConfigurationChange(Configuration &configuration);

    void onConnectButtonClicked();
    void onSettingsButtonClicked();

    void onListenCheckboxStateChanged(int newState);

private:
    ConnectionDialog* createConnectionDialog();
    void initializeSessionCreator();
    Configuration loadConfiguration();

    Ui::MainWindow *ui_;

    std::unique_ptr<ChatSessionCreator> sessionCreator_;
    Configuration configuration_;

    QString host_;
    int port_;
};
#endif // MAINWINDOW_H
