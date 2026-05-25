#include <QtTest>
#include <vector>
#include <memory>
#include "../../src/service/impl/TrashService.h"
#include "../../src/repository/SqliteNoteRepository.h"
#include "../../src/model/Note.h"

class TrashServiceTests : public QObject {
    Q_OBJECT
private slots:
    void trashRestoreAndPurge() {
        SqliteNoteRepository repo(":memory:");
        QVERIFY(repo.isConnected());

        Note note("text", "Trash Test");
        QVERIFY(repo.save(note));
        qint64 nid = note.noteId();
        QVERIFY(nid > 0);

        TrashService svc(&repo);

        // Trash
        QVERIFY(svc.trashNote(nid));
        auto trashed = svc.getTrashedNotes();
        QVERIFY(!trashed.empty());

        // Restore
        QVERIFY(svc.restoreNote(nid));
        auto trashed2 = svc.getTrashedNotes();
        QCOMPARE(trashed2.size(), size_t(0));

        // Trash again and purge
        QVERIFY(svc.trashNote(nid));
        QVERIFY(svc.purgeNote(nid));
        Note* after = repo.getById(nid);
        QVERIFY(after == nullptr);
    }
};

QTEST_MAIN(TrashServiceTests)
#include "trash_service_tests.moc"
