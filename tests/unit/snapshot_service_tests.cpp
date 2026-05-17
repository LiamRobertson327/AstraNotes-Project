#include <QtTest>
#include "../../src/service/impl/SnapshotService.h"
#include "../../src/repository/SqliteNoteRepository.h"
#include "../../src/model/Note.h"

class SnapshotServiceTests : public QObject {
    Q_OBJECT
private slots:
    void saveAndPruneSnapshots() {
        SqliteNoteRepository repo(":memory:");
        QVERIFY(repo.isConnected());

        // Create and persist a note
        Note note("text", "Snapshot Test");
        QVERIFY(repo.save(note));
        qint64 nid = note.noteId();
        QVERIFY(nid > 0);

        SnapshotService svc(&repo);

        // Save three snapshots
        QVERIFY(svc.saveSnapshot(note));
        QVERIFY(svc.saveSnapshot(note));
        QVERIFY(svc.saveSnapshot(note));

        auto snaps = svc.getSnapshotsByNoteId(nid);
        QVERIFY(snaps.size() >= 3);

        // Enforce a limit of 2
        svc.enforceSnapshotLimit(nid, 2);
        auto pruned = svc.getSnapshotsByNoteId(nid);
        QCOMPARE(pruned.size(), 2);

        // Clean up allocated snapshots
        qDeleteAll(snaps);
        qDeleteAll(pruned);
    }
};

QTEST_MAIN(SnapshotServiceTests)
#include "snapshot_service_tests.moc"
