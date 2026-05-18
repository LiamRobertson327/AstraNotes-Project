#include "NoteListController.h"

#include "../service/interfaces/INoteService.h"
#include "../model/Note.h"

#include <QAbstractItemView>
#include <QScrollBar>
#include <QDebug>

NoteListController::NoteListController(QListWidget *noteList, INoteService *noteService, QObject *parent)
    : QObject(parent),
      m_noteList(noteList),
      m_noteService(noteService),
      m_pageSize(50),
      m_currentOffset(0),
      m_allLoaded(false) {
    if (m_noteList) {
        connect(m_noteList, &QListWidget::itemClicked, this, &NoteListController::onItemClicked);
        if (m_noteList->verticalScrollBar()) {
            connect(m_noteList->verticalScrollBar(), &QScrollBar::valueChanged,
                    this, &NoteListController::onScrollBarValueChanged);
        }
    }
}

void NoteListController::reload() {
    if (!m_noteList) {
        return;
    }

    m_noteList->clear();
    m_currentOffset = 0;
    m_allLoaded = false;
    loadPage(0);
}

void NoteListController::loadPage(int offset) {
    if (!m_noteList || !m_noteService || m_allLoaded) {
        return;
    }

    QVector<Note *> page = m_noteService->searchByTitlePaged(QString(), m_pageSize, offset);
    qDebug() << "[NoteListController::loadPage] Loaded" << page.size() << "notes for offset" << offset;

    for (Note *note : page) {
        if (!note) {
            continue;
        }

        QListWidgetItem *item = new QListWidgetItem(note->displayText(), m_noteList);
        item->setData(Qt::UserRole, note->noteId());
        delete note;
    }

    if (page.size() < m_pageSize) {
        m_allLoaded = true;
    } else {
        m_currentOffset += page.size();
    }
}

void NoteListController::onItemClicked(QListWidgetItem *item) {
    if (!item) {
        return;
    }

    emit noteSelected(item->data(Qt::UserRole).toLongLong());
}

void NoteListController::onScrollBarValueChanged(int value) {
    Q_UNUSED(value);

    if (!m_noteList || !m_noteList->verticalScrollBar() || m_allLoaded) {
        return;
    }

    QScrollBar *scrollBar = m_noteList->verticalScrollBar();
    if (scrollBar->value() >= scrollBar->maximum() - 120) {
        loadPage(m_currentOffset);
    }
}

void NoteListController::trashSelectedNotes() {
    if (!m_noteList || !m_noteService) return;

    QList<QListWidgetItem*> items = m_noteList->selectedItems();
    if (items.isEmpty()) return;

    int moved = 0;
    for (QListWidgetItem *item : items) {
        qint64 noteId = item->data(Qt::UserRole).toLongLong();
        if (m_noteService && m_noteService->trashNote(noteId)) {
            ++moved;
        }
    }

    // Refresh list after trashing
    reload();
}

void NoteListController::noteSaved(Note *note) {
    if (!m_noteList || !note) return;

    // Update existing item text or insert new item at top.
    // Keeping the saved note at index 0 matches the modified_at DESC sort.
    QListWidgetItem *currentItem = m_noteList->currentItem();
    bool found = false;
    for (int i = 0; i < m_noteList->count(); ++i) {
        QListWidgetItem *item = m_noteList->item(i);
        if (item->data(Qt::UserRole).toLongLong() == note->noteId()) {
            const bool wasCurrent = (item == currentItem);
            const bool wasSelected = item->isSelected();
            item->setText(note->displayText());
            QListWidgetItem *movedItem = m_noteList->takeItem(i);
            if (movedItem) {
                m_noteList->insertItem(0, movedItem);
                movedItem->setSelected(wasSelected);
                if (wasCurrent || wasSelected) {
                    m_noteList->setCurrentItem(movedItem);
                    m_noteList->scrollToItem(movedItem, QAbstractItemView::PositionAtTop);
                }
            }
            found = true;
            break;
        }
    }
    if (!found) {
        QListWidgetItem *newItem = new QListWidgetItem(note->displayText());
        newItem->setData(Qt::UserRole, note->noteId());
        m_noteList->insertItem(0, newItem);
        if (currentItem && currentItem->data(Qt::UserRole).toLongLong() == note->noteId()) {
            m_noteList->setCurrentItem(newItem);
            newItem->setSelected(true);
            m_noteList->scrollToItem(newItem, QAbstractItemView::PositionAtTop);
        }
    }
}
