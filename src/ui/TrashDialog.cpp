#include "TrashDialog.h"
#include "../service/interfaces/ITrashService.h"
#include "../model/Note.h"
#include <QDateTime>
#include <QMessageBox>

TrashDialog::TrashDialog(ITrashService *trashService, int retentionDays_, QWidget *parent)
    : QDialog(parent), trashService(trashService), retentionDays(retentionDays_) {
    setWindowTitle("Trashed Notes");
    setMinimumSize(600, 400);

    QVBoxLayout *root = new QVBoxLayout(this);
    QLabel *info = new QLabel(QString("Select trashed notes to restore or purge. Auto-purge after %1 days.").arg(retentionDays));
    root->addWidget(info);

    listWidget = new QListWidget(this);
    listWidget->setSelectionMode(QAbstractItemView::NoSelection);
    root->addWidget(listWidget, 1);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    restoreButton = new QPushButton("Restore Selected");
    purgeButton = new QPushButton("Purge Selected Permanently");
    closeButton = new QPushButton("Close");

    btnLayout->addWidget(restoreButton);
    btnLayout->addWidget(purgeButton);
    btnLayout->addStretch();
    btnLayout->addWidget(closeButton);

    root->addLayout(btnLayout);

    connect(restoreButton, &QPushButton::clicked, this, &TrashDialog::onRestoreSelected);
    connect(purgeButton, &QPushButton::clicked, this, &TrashDialog::onPurgeSelected);
    connect(closeButton, &QPushButton::clicked, this, &TrashDialog::close);

    populateList();
}

void TrashDialog::populateList() {
    listWidget->clear();
    QVector<Note*> trashed = trashService->getTrashedNotes();

    for (Note *n : trashed) {
        QString title = n->title().isEmpty() ? "(untitled)" : n->title();
        QString prefix;
        if (n->isSecured()) {
            prefix = QString::fromUtf8("🔒 ");
        }
        QDateTime trashedAt = n->lastModified();
        QString purgeAt = purgeDateString(trashedAt);

        QString itemText = QString("%1%2\nCreated: %3 | Modified: %4 | Purge At: %5")
            .arg(prefix)
            .arg(title)
            .arg(n->createdAt().toLocalTime().toString(Qt::ISODate))
            .arg(n->lastModified().toLocalTime().toString(Qt::ISODate))
            .arg(purgeAt);

        QListWidgetItem *item = new QListWidgetItem(itemText, listWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
        item->setData(Qt::UserRole, QVariant::fromValue(n->noteId()));

        delete n; // Caller ownership transferred to us; free the Note pointers
    }
}

QString TrashDialog::purgeDateString(const QDateTime &trashedAt) const {
    if (!trashedAt.isValid()) {
        return "Unknown";
    }
    QDateTime purgeAt = trashedAt.addDays(retentionDays);
    return purgeAt.toLocalTime().toString(Qt::ISODate);
}

void TrashDialog::onRestoreSelected() {
    QList<qint64> ids;
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            ids.append(item->data(Qt::UserRole).toLongLong());
        }
    }

    if (ids.isEmpty()) {
        QMessageBox::information(this, "Restore", "No items selected to restore.");
        return;
    }

    int restored = 0;
    for (qint64 id : ids) {
        if (trashService->restoreNote(id)) {
            ++restored;
        }
    }

    QMessageBox::information(this, "Restore", QString("Restored %1 notes.").arg(restored));
    populateList();
    emit changesApplied();
}

void TrashDialog::onPurgeSelected() {
    QList<qint64> ids;
    for (int i = 0; i < listWidget->count(); ++i) {
        QListWidgetItem *item = listWidget->item(i);
        if (item->checkState() == Qt::Checked) {
            ids.append(item->data(Qt::UserRole).toLongLong());
        }
    }

    if (ids.isEmpty()) {
        QMessageBox::information(this, "Purge", "No items selected to purge.");
        return;
    }

    int purged = 0;
    for (qint64 id : ids) {
        if (trashService->purgeNote(id)) {
            ++purged;
        }
    }

    QMessageBox::information(this, "Purge", QString("Permanently purged %1 notes.").arg(purged));
    populateList();
    emit changesApplied();
}
