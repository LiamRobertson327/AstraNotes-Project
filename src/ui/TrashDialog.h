#ifndef TRASHDIALOG_H
#define TRASHDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateTime>

class ITrashService;

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDateTime>

class ITrashService;

class TrashDialog : public QDialog {
    Q_OBJECT
public:
    TrashDialog(ITrashService *trashService, int retentionDays = 14, QWidget *parent = nullptr);

signals:
    void changesApplied(); // Emitted when restore or purge completed so callers can refresh

private slots:
    void onRestoreSelected();
    void onPurgeSelected();

private:
    void populateList();
    QString purgeDateString(const QDateTime &trashedAt) const;

    ITrashService *trashService;
    int retentionDays;
    QListWidget *listWidget;
    QPushButton *restoreButton;
    QPushButton *purgeButton;
    QPushButton *closeButton;
};

#endif // TRASHDIALOG_H