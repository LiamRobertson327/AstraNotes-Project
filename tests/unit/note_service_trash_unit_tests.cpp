#include <QtTest>
#include "../../src/service/impl/NoteService.h"
#include "../../src/repository/INoteRepository.h"
#include "../../src/model/Note.h"

// Minimal mock implementing INoteRepository for unit testing NoteService
class MockRepo : public INoteRepository {
public:
    bool trashCalled = false;
    qint64 lastTrashedId = -1;

    bool save(Note &note) override { Q_UNUSED(note); return true; }
    Note* getById(qint64 id) override { Q_UNUSED(id); return nullptr; }
    Note* getById(qint64 id, const QString &password, bool *wrongPassword = nullptr) override { Q_UNUSED(id); Q_UNUSED(password); if (wrongPassword) *wrongPassword = false; return nullptr; }
    QVector<Note*> getAll() override { return {}; }
    bool deleteById(qint64 id) override { Q_UNUSED(id); return true; }
    bool update(const Note &note) override { Q_UNUSED(note); return true; }
    QVector<Note*> searchByTitle(const QString &query) override { Q_UNUSED(query); return {}; }
    QVector<Note*> searchByContent(const QString &query) override { Q_UNUSED(query); return {}; }
    QVector<Note*> getTrashedNotes() override { return {}; }
    bool trashNote(qint64 id) override { trashCalled = true; lastTrashedId = id; return true; }
    bool restoreNote(qint64 id) override { Q_UNUSED(id); return true; }
    bool purgeTrashedNotes(int olderThanDays = 14) override { Q_UNUSED(olderThanDays); return true; }
    bool save(Note &note, const QString &password) override { Q_UNUSED(note); Q_UNUSED(password); return true; }
    int countActiveNotes() const override { return 0; }
    int countActiveNotesByType(const QString &typeId) const override { Q_UNUSED(typeId); return 0; }
    QVector<Note*> searchByTitlePaged(const QString &query, int limit, int offset) override { Q_UNUSED(query); Q_UNUSED(limit); Q_UNUSED(offset); return {}; }
    int countTitleMatches(const QString &query) override { Q_UNUSED(query); return 0; }
    bool isConnected() const override { return true; }
    // Snapshot stubs added to satisfy new INoteRepository methods
    bool saveSnapshot(class Snapshot &snapshot) override { Q_UNUSED(snapshot); return true; }
    bool saveSnapshot(class Snapshot &snapshot, const QString &password) override { Q_UNUSED(snapshot); Q_UNUSED(password); return true; }
    QVector<class Snapshot*> getSnapshotsByNoteId(qint64 noteId) override { Q_UNUSED(noteId); return {}; }
    QVector<class Snapshot*> getSnapshotsByNoteId(qint64 noteId, const QString &password) override { Q_UNUSED(noteId); Q_UNUSED(password); return {}; }
    class Snapshot* getSnapshotById(qint64 snapshotId) override { Q_UNUSED(snapshotId); return nullptr; }
    class Snapshot* getSnapshotById(qint64 snapshotId, const QString &password, bool *wrongPassword = nullptr) override { Q_UNUSED(snapshotId); Q_UNUSED(password); if (wrongPassword) *wrongPassword = false; return nullptr; }
    bool deleteSnapshotById(qint64 snapshotId) override { Q_UNUSED(snapshotId); return true; }
    bool deleteOldestSnapshotForNote(qint64 noteId) override { Q_UNUSED(noteId); return true; }
    bool pruneOldSnapshots(qint64 noteId) override { Q_UNUSED(noteId); return true; }
};

class NoteServiceTrashUnitTests : public QObject {
    Q_OBJECT

private slots:
    void trashNote_callsRepository() {
        MockRepo mock;
        NoteService svc(&mock);

        QVERIFY(!mock.trashCalled);
        QVERIFY(svc.trashNote(1234));
        QVERIFY(mock.trashCalled);
        QCOMPARE(mock.lastTrashedId, (qint64)1234);
    }
};

QTEST_MAIN(NoteServiceTrashUnitTests)
#include "note_service_trash_unit_tests.moc"
