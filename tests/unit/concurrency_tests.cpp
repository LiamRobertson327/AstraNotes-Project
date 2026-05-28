#include <QtTest>
#include <thread>
#include <mutex>
#include <atomic>
#include "../../src/service/impl/NoteService.h"
#include "../../src/repository/INoteRepository.h"
#include "../../src/model/Snapshot.h"

class FakeConcurrentRepo : public INoteRepository {
public:
    bool save(Note &note) override {
        std::lock_guard<std::mutex> lk(mtx);
        auto n = std::make_unique<Note>(note.typeId(), note.title());
        n->setNoteId(++lastId);
        storage.push_back(std::move(n));
        return true;
    }
    std::unique_ptr<Note> getById(qint64 id) override { Q_UNUSED(id); return nullptr; }
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
    int countActiveNotes() const override { std::lock_guard<std::mutex> lk(mtx); return (int)storage.size(); }
    int countActiveNotesByType(const QString &) const override { return 0; }
    std::vector<std::unique_ptr<Note>> searchByTitlePaged(const QString &, int, int) override { return {}; }
    int countTitleMatches(const QString &) override { return 0; }
    bool isConnected() const override { return true; }

private:
    mutable std::mutex mtx;
    std::vector<std::unique_ptr<Note>> storage;
    std::atomic<qint64> lastId{0};
};

class ConcurrencyTests : public QObject {
    Q_OBJECT

private slots:
    void concurrentSaves() {
        FakeConcurrentRepo repo;
        NoteService svc(&repo);

        const int threads = 8;
        const int perThread = 50;
        std::vector<std::thread> ths;
        for (int t = 0; t < threads; ++t) {
            ths.emplace_back([&svc, perThread]() {
                for (int i = 0; i < perThread; ++i) {
                    auto n = svc.createNote("plaintext", "ct");
                    QString err;
                    svc.saveNote(*n, QString(), &err);
                }
            });
        }
        for (auto &t : ths) t.join();

        QCOMPARE(repo.countActiveNotes(), threads * perThread);
    }
};

QTEST_MAIN(ConcurrencyTests)
#include "concurrency_tests.moc"
