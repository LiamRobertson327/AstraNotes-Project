#include <QtTest>
#include "../../src/service/impl/NoteService.h"
#include "../../src/repository/INoteRepository.h"
#include "../../src/model/Snapshot.h"

class FakeEncryptedRepo : public INoteRepository {
public:
    bool save(Note &note) override { Q_UNUSED(note); return true; }
    std::unique_ptr<Note> getById(qint64) override { return nullptr; }
    std::unique_ptr<Note> getById(qint64, const QString &password, bool *wrongPassword = nullptr) override {
        if (wrongPassword) *wrongPassword = true;
        return nullptr;
    }
    // stub remaining methods with safe defaults
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
    bool isConnected() const override { return true; }
};

class EncryptionFailureTests : public QObject {
    Q_OBJECT

private slots:
    void wrongPasswordSetsFlag() {
        FakeEncryptedRepo repo;
        NoteService svc(&repo);
        bool wrong = false;
        auto note = svc.loadNote(1, "badpass", &wrong);
        QVERIFY(!note);
        QVERIFY(wrong);
    }
};

QTEST_MAIN(EncryptionFailureTests)
#include "encryption_failure_tests.moc"
