#include <QtTest>
#include <QTemporaryDir>
#include <QDir>

#include "../../src/model/Note.h"
#include "../../src/model/Snapshot.h"
#include "../../src/repository/SqliteNoteRepository.h"

class RepositoryRoundtripIntegrationTests : public QObject {
    Q_OBJECT

private slots:
    void titleSearchCountsAndTypes_roundTrip() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        SqliteNoteRepository repo(QDir(tempDir.path()).filePath("notes.db"));
        QVERIFY(repo.isConnected());

        Note first("plaintext", "Project Alpha");
        first.setContent("alpha content");
        QVERIFY(repo.save(first));

        Note second("plaintext", "Project Beta");
        second.setContent("beta content");
        QVERIFY(repo.save(second));

        Note third("markdown", "Meeting Notes");
        third.setContent("meeting content");
        QVERIFY(repo.save(third));

        QCOMPARE(repo.countActiveNotes(), 3);
        QCOMPARE(repo.countActiveNotesByType("plaintext"), 2);
        QCOMPARE(repo.countActiveNotesByType("markdown"), 1);
        QCOMPARE(repo.countTitleMatches("Project"), 2);

        QVector<Note*> page = repo.searchByTitlePaged("Project", 10, 0);
        QCOMPARE(page.size(), 2);
        QVERIFY(page[0]->title().contains("Project"));
        QVERIFY(page[1]->title().contains("Project"));

        qDeleteAll(page);
    }

    void snapshotRoundTrip_preservesLatestContent() {
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        SqliteNoteRepository repo(QDir(tempDir.path()).filePath("snapshots.db"));
        QVERIFY(repo.isConnected());

        Note note("plaintext", "Snapshot Roundtrip");
        note.setContent("version one");
        QVERIFY(repo.save(note));

        Snapshot first(note.noteId(), note.title(), note.content());
        QVERIFY(repo.saveSnapshot(first));

        QTest::qWait(1100);

        note.setContent("version two");
        QVERIFY(repo.save(note));

        Snapshot second(note.noteId(), note.title(), note.content());
        QVERIFY(repo.saveSnapshot(second));

        QVector<Snapshot*> snapshots = repo.getSnapshotsByNoteId(note.noteId());
        QCOMPARE(snapshots.size(), 2);
        QCOMPARE(snapshots[0]->content(), QString("version two"));
        QCOMPARE(snapshots[1]->content(), QString("version one"));

        Snapshot* byId = repo.getSnapshotById(second.snapshotId());
        QVERIFY(byId != nullptr);
        QCOMPARE(byId->title(), note.title());
        QCOMPARE(byId->content(), QString("version two"));

        qDeleteAll(snapshots);
        delete byId;
    }
};

QTEST_MAIN(RepositoryRoundtripIntegrationTests)
#include "repository_roundtrip_integration_tests.moc"