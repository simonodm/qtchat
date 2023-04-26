#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include "configuration.h"

#include <QDialog>

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsWindow(Configuration config, QWidget *parent = nullptr);
    ~SettingsWindow();

signals:
    void configurationChanged(Configuration &configuration);

private slots:
    void onOpenPublicKeyFileButtonClicked();
    void onOpenPrivateKeyFileButtonClicked();
    void onSaveButtonClicked();

    void onPublicKeyFileSelected(const QString &file);
    void onPrivateKeyFileSelected(const QString &file);

private:
    Ui::SettingsWindow *ui;

    Configuration config_;
};

#endif // SETTINGSWINDOW_H
