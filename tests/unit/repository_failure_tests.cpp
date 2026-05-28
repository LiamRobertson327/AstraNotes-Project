#include <QtTest>
#include "../../src/service/impl/NoteService.h"
#include "../../src/repository/INoteRepository.h"
#include "../../src/model/Snapshot.h"

class FakeFailRepo : public INoteRepository {
public:
    bool save(Note &note) override { Q_UNUSED(note); return false; }
    std::unique_ptr<Note> getById(qint64) override { return nullptr; }
    std::unique_ptr<Note> getById(qint64, const QString &, bool *) override { return nullptr; }
    std::vector<std::unique_ptr<Note>> getAll() override { return {}; }
    bool deleteById(qint64) override { return false; }
    bool update(const Note &) override { return false; }
    std::vector<std::unique_ptr<Note>> searchByTitle(const QString &) override { return {}; }
    std::vector<std::unique_ptr<Note>> searchByContent(const QString &) override { return {}; }
    bool saveSnapshot(class Snapshot &) override { return false; }
    bool saveSnapshot(class Snapshot &, const QString &) override { return false; }
    std::vector<std::unique_ptr<class Snapshot>> getSnapshotsByNoteId(qint64) override { return {}; }
    std::vector<std::unique_ptr<class Snapshot>> getSnapshotsByNoteId(qint64, const QString &) override { return {}; }
    std::unique_ptr<class Snapshot> getSnapshotById(qint64) override { return nullptr; }
    std::unique_ptr<class Snapshot> getSnapshotById(qint64, const QString &, bool *) override { return nullptr; }
    bool deleteSnapshotById(qint64) override { return false; }
    bool deleteOldestSnapshotForNote(qint64) override { return false; }
    bool pruneOldSnapshots(qint64) override { return false; }
    std::vector<std::unique_ptr<Note>> getTrashedNotes() override { return {}; }
    std::vector<std::unique_ptr<Note>> getTrashedNotes(int, int) override { return {}; }
    int countTrashedNotes() override { return 0; }
    bool isNoteTrashed(qint64) override { return false; }
    bool trashNote(qint64) override { return false; }
    bool restoreNote(qint64) override { return false; }
    bool purgeTrashedNotes(int) override { return false; }
    int countActiveNotes() const override { return 0; }
    int countActiveNotesByType(const QString &) const override { return 0; }
    std::vector<std::unique_ptr<Note>> searchByTitlePaged(const QString &, int, int) override { return {}; }
    int countTitleMatches(const QString &) override { return 0; }
    bool isConnected() const override { return false; }
};

class RepositoryFailureTests : public QObject {
    Q_OBJECT

private slots:
    void saveNoteRepoUnavailable() {
        FakeFailRepo repo;
        NoteService svc(&repo);
        Note n("plaintext", "t");
        QString err;
        bool ok = svc.saveNote(n, QString(), &err);
        QVERIFY(!ok);
        QVERIFY(!err.isEmpty());
    }

    void loadNoteRepoUnavailable() {
        FakeFailRepo repo;
        NoteService svc(&repo);
        QString err;
        auto note = svc.loadNote(123, QString(), nullptr, &err);
        QVERIFY(!note);
        QVERIFY(!err.isEmpty());
    }
};

QTEST_MAIN(RepositoryFailureTests)
#include "repository_failure_tests.moc"
