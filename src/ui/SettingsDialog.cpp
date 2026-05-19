#include "SettingsDialog.h"
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QWheelEvent>

class ArrowOnlySpinBox : public QSpinBox {
public:
    explicit ArrowOnlySpinBox(QWidget *parent = nullptr) : QSpinBox(parent) {}

protected:
    void keyPressEvent(QKeyEvent *event) override {
        switch (event->key()) {
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            QSpinBox::keyPressEvent(event);
            break;
        default:
            event->ignore();
            break;
        }
    }

    void wheelEvent(QWheelEvent *event) override {
        event->ignore();
    }
};

SettingsDialog::SettingsDialog(int retentionDaysInit, bool autoPurgeEnabledInit, int autoSaveDebounceMs,
                               bool defaultEncryptionEnabledInit, QWidget *parent)
    : QDialog(parent) {
    setWindowTitle("Settings");
    setMinimumSize(380, 200);

    QVBoxLayout *root = new QVBoxLayout(this);

    QLabel *retLabel = new QLabel("Retention (days before auto-purge):");
    retentionSpin = new ArrowOnlySpinBox(this);
    retentionSpin->setMinimum(1);
    retentionSpin->setMaximum(60);
    retentionSpin->setValue(retentionDaysInit);
    retentionSpin->setKeyboardTracking(false);

    QHBoxLayout *retLayout = new QHBoxLayout();
    retLayout->addWidget(retLabel);
    retLayout->addWidget(retentionSpin);
    root->addLayout(retLayout);

    autoPurgeCheck = new QCheckBox("Enable automatic purge (daily)");
    autoPurgeCheck->setChecked(autoPurgeEnabledInit);
    root->addWidget(autoPurgeCheck);

    defaultEncryptionCheck = new QCheckBox("Enable default encryption for new notes");
    defaultEncryptionCheck->setChecked(defaultEncryptionEnabledInit);
    root->addWidget(defaultEncryptionCheck);

    QLabel *debounceLabel = new QLabel("Auto-save debounce interval (ms):");
    autoSaveDebounceSpinner = new ArrowOnlySpinBox(this);
    autoSaveDebounceSpinner->setMinimum(500);
    autoSaveDebounceSpinner->setMaximum(10000);
    autoSaveDebounceSpinner->setSingleStep(100);
    autoSaveDebounceSpinner->setValue(autoSaveDebounceMs);
    autoSaveDebounceSpinner->setKeyboardTracking(false);

    QHBoxLayout *debounceLayout = new QHBoxLayout();
    debounceLayout->addWidget(debounceLabel);
    debounceLayout->addWidget(autoSaveDebounceSpinner);
    root->addLayout(debounceLayout);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *ok = new QPushButton("OK");
    QPushButton *cancel = new QPushButton("Cancel");
    btnLayout->addStretch();
    btnLayout->addWidget(ok);
    btnLayout->addWidget(cancel);
    root->addLayout(btnLayout);

    connect(ok, &QPushButton::clicked, this, &SettingsDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &SettingsDialog::reject);
}

int SettingsDialog::retentionDays() const {
    return retentionSpin->value();
}

bool SettingsDialog::autoPurgeEnabled() const {
    return autoPurgeCheck->isChecked();
}

int SettingsDialog::autoSaveDebounceMs() const {
    return autoSaveDebounceSpinner->value();
}

bool SettingsDialog::defaultEncryptionEnabled() const {
    return defaultEncryptionCheck ? defaultEncryptionCheck->isChecked() : false;
}
