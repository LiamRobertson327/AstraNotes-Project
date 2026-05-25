#ifndef NOTELISTCONTROLLER_H
#define NOTELISTCONTROLLER_H

#include <QObject>
#include <QListWidget>
#include <QVector>

class INoteService;
class Note;

/// NoteListController owns sidebar paging and note selection plumbing.
/// It keeps MainWindow focused on UI orchestration while handling list loading.
class NoteListController : public QObject {
    Q_OBJECT
public:
    explicit NoteListController(QListWidget *noteList, INoteService *noteService, QObject *parent = nullptr);

    void reload();
    void trashSelectedNotes();
    void noteSaved(class Note *note);

signals:
    void noteSelected(qint64 noteId);

private slots:
    void onItemClicked(QListWidgetItem *item);
    void onScrollBarValueChanged(int value);

private:
    void loadPage(int offset);

    QListWidget *m_noteList;
    INoteService *m_noteService;
    int m_pageSize;
    int m_currentOffset;
    bool m_allLoaded;
    bool m_loading;
};

#endif // NOTELISTCONTROLLER_H
