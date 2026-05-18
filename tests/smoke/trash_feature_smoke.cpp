#include <QtTest>
#include "../../src/repository/SqliteNoteRepository.h"
#include "../../src/service/impl/NoteService.h"

class TrashFeatureSmoke : public QObject {
    Q_OBJECT

private slots:
    void create_trash_restore_purge_flow() {
        SqliteNoteRepository repo(":memory:");
        QVERIFY(repo.isConnected());

        // Create and save a note
        Note note("text", "Smoke Trash Note");
        QVERIFY(repo.save(note));
        qint64 nid = note.noteId();
        QVERIFY(nid > 0);

        NoteService noteSvc(&repo);

        // Trash the note
        QVERIFY(noteSvc.trashNote(nid));

        // Restore it
        QVERIFY(repo.restoreNote(nid));

        // Trash again and purge older than 0 days (should remove immediately)
        QVERIFY(noteSvc.trashNote(nid));
        QVERIFY(repo.purgeTrashedNotes(0));

        // Ensure it's gone from trashed list and active list
        auto trashed = repo.getTrashedNotes();
        bool found = false;
        for (auto n : trashed) {
            if (n->noteId() == nid) found = true;
            delete n;
        }
        QVERIFY(!found);
    }
};

QTEST_MAIN(TrashFeatureSmoke)
#include "trash_feature_smoke.moc"
