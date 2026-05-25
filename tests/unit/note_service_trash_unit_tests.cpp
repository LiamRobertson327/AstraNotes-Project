#include <QtTest>
#include <vector>
#include <memory>
#include "../../src/service/impl/NoteService.h"
#include "../../src/repository/INoteRepository.h"
#include "../../src/model/Note.h"
#include "../../src/model/Snapshot.h"

using SnapshotPtr = std::unique_ptr<Snapshot>;
using SnapshotList = std::vector<SnapshotPtr>;

// Minimal mock implementing INoteRepository for unit testing NoteService
class MockRepo : public INoteRepository {
public:
    bool trashCalled = false;
    qint64 lastTrashedId = -1;

    bool save(Note &note) override { Q_UNUSED(note); return true; }
    std::unique_ptr<Note> getById(qint64 id) override { Q_UNUSED(id); return {}; }
    std::unique_ptr<Note> getById(qint64 id, const QString &password, bool *wrongPassword = nullptr) override { Q_UNUSED(id); Q_UNUSED(password); if (wrongPassword) *wrongPassword = false; return {}; }
    std::vector<std::unique_ptr<Note>> getAll() override { return {}; }
    bool deleteById(qint64 id) override { Q_UNUSED(id); return true; }
    bool update(const Note &note) override { Q_UNUSED(note); return true; }
    std::vector<std::unique_ptr<Note>> searchByTitle(const QString &query) override { Q_UNUSED(query); return {}; }
    std::vector<std::unique_ptr<Note>> searchByContent(const QString &query) override { Q_UNUSED(query); return {}; }
    std::vector<std::unique_ptr<Note>> getTrashedNotes() override { return {}; }
    std::vector<std::unique_ptr<Note>> getTrashedNotes(int limit, int offset) override { Q_UNUSED(limit); Q_UNUSED(offset); return {}; }
    int countTrashedNotes() override { return 0; }
    bool isNoteTrashed(qint64 id) override { Q_UNUSED(id); return true; }
    bool trashNote(qint64 id) override { trashCalled = true; lastTrashedId = id; return true; }
    bool restoreNote(qint64 id) override { Q_UNUSED(id); return true; }
    bool purgeTrashedNotes(int olderThanDays = 14) override { Q_UNUSED(olderThanDays); return true; }
    bool save(Note &note, const QString &password) override { Q_UNUSED(note); Q_UNUSED(password); return true; }
    int countActiveNotes() const override { return 0; }
    int countActiveNotesByType(const QString &typeId) const override { Q_UNUSED(typeId); return 0; }
    std::vector<std::unique_ptr<Note>> searchByTitlePaged(const QString &query, int limit, int offset) override { Q_UNUSED(query); Q_UNUSED(limit); Q_UNUSED(offset); return {}; }
    int countTitleMatches(const QString &query) override { Q_UNUSED(query); return 0; }
    bool isConnected() const override { return true; }
    // Snapshot stubs added to satisfy new INoteRepository methods
    bool saveSnapshot(class Snapshot &snapshot) override { Q_UNUSED(snapshot); return true; }
    bool saveSnapshot(class Snapshot &snapshot, const QString &password) override { Q_UNUSED(snapshot); Q_UNUSED(password); return true; }
    SnapshotList getSnapshotsByNoteId(qint64 noteId) override { Q_UNUSED(noteId); return {}; }
    SnapshotList getSnapshotsByNoteId(qint64 noteId, const QString &password) override { Q_UNUSED(noteId); Q_UNUSED(password); return {}; }
    SnapshotPtr getSnapshotById(qint64 snapshotId) override { Q_UNUSED(snapshotId); return {}; }
    SnapshotPtr getSnapshotById(qint64 snapshotId, const QString &password, bool *wrongPassword = nullptr) override { Q_UNUSED(snapshotId); Q_UNUSED(password); if (wrongPassword) *wrongPassword = false; return {}; }
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
