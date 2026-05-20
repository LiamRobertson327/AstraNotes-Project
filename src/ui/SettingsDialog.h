#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class ArrowOnlySpinBox;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    SettingsDialog(int retentionDays, bool autoPurgeEnabled, int autoSaveDebounceMs,
                   bool defaultEncryptionEnabled = false, QWidget *parent = nullptr);

    int retentionDays() const;
    bool autoPurgeEnabled() const;
    int autoSaveDebounceMs() const;
    bool defaultEncryptionEnabled() const;

private:
    QSpinBox *retentionSpin;
    QCheckBox *autoPurgeCheck;
    QSpinBox *autoSaveDebounceSpinner;
    QCheckBox *defaultEncryptionCheck;
};

#endif // SETTINGSDIALOG_H
