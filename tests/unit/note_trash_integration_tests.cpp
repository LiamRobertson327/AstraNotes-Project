#include <QtTest>
#include <vector>
#include <memory>
#include "../../src/service/impl/NoteService.h"
#include "../../src/repository/SqliteNoteRepository.h"
#include "../../src/model/Note.h"

class NoteTrashIntegrationTests : public QObject {
    Q_OBJECT

private slots:
    void trashNote_movesToTrash() {
        SqliteNoteRepository repo(":memory:");
        QVERIFY(repo.isConnected());

        // Create and persist a note
        Note note("text", "Trash Integration Test");
        QVERIFY(repo.save(note));
        qint64 nid = note.noteId();
        QVERIFY(nid > 0);

        NoteService svc(&repo);
        // Ensure note is active initially
        int beforeActive = repo.countActiveNotes();
        QVERIFY(beforeActive >= 1);

        QVERIFY(svc.trashNote(nid));

        // After trashing, active count should decrease
        int afterActive = repo.countActiveNotes();
        QVERIFY(afterActive == beforeActive - 1);

        // Trashed listing should contain the note
        auto trashed = repo.getTrashedNotes();
        bool found = false;
        for (const auto &n : trashed) {
            if (n->noteId() == nid) found = true;
        }
        QVERIFY(found);
    }
};

QTEST_MAIN(NoteTrashIntegrationTests)
#include "note_trash_integration_tests.moc"
