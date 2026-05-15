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
    SettingsDialog(int retentionDays, bool autoPurgeEnabled, int autoSaveDebounceMs, QWidget *parent = nullptr);

    int retentionDays() const;
    bool autoPurgeEnabled() const;
    int autoSaveDebounceMs() const;

private:
    QSpinBox *retentionSpin;
    QCheckBox *autoPurgeCheck;
    QSpinBox *autoSaveDebounceSpinner;
};

#endif // SETTINGSDIALOG_H
