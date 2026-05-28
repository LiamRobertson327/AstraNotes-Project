#include <QtTest>
#include "../../src/service/impl/NoteService.h"
#include "../../src/repository/INoteRepository.h"
#include "../../src/model/Snapshot.h"

class FakeSearchRepo : public INoteRepository {
public:
    FakeSearchRepo() {
        auto a = std::make_unique<Note>("plaintext", "emoji 😊");
        a->setContent("This is a note with emoji 😊 and accents: café");
        storage.push_back(std::move(a));

        auto b = std::make_unique<Note>("plaintext", "longline");
        QString longc(5000, 'x');
        b->setContent(longc);
        storage.push_back(std::move(b));
    }

    bool save(Note &note) override { Q_UNUSED(note); return true; }
    std::unique_ptr<Note> getById(qint64) override { return nullptr; }
    std::unique_ptr<Note> getById(qint64, const QString &, bool *) override { return nullptr; }
    std::vector<std::unique_ptr<Note>> getAll() override { return {}; }
    bool deleteById(qint64) override { return false; }
    bool update(const Note &) override { return false; }
    std::vector<std::unique_ptr<Note>> searchByTitle(const QString &query) override {
        std::vector<std::unique_ptr<Note>> out;
        for (auto &n : storage) {
            if (n->title().contains(query, Qt::CaseInsensitive)) {
                out.push_back(std::make_unique<Note>(n->typeId(), n->title()));
            }
        }
        return out;
    }
    std::vector<std::unique_ptr<Note>> searchByContent(const QString &query) override {
        std::vector<std::unique_ptr<Note>> out;
        for (auto &n : storage) {
            if (n->content().contains(query, Qt::CaseInsensitive)) {
                out.push_back(std::make_unique<Note>(n->typeId(), n->title()));
            }
        }
        return out;
    }
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
    int countActiveNotes() const override { return (int)storage.size(); }
    int countActiveNotesByType(const QString &) const override { return 0; }
    std::vector<std::unique_ptr<Note>> searchByTitlePaged(const QString &query, int, int) override { return searchByTitle(query); }
    int countTitleMatches(const QString &query) override { return (int)searchByTitle(query).size(); }
    bool isConnected() const override { return true; }

private:
    std::vector<std::unique_ptr<Note>> storage;
};

class SearchEdgecasesTests : public QObject {
    Q_OBJECT

private slots:
    void unicodeAndEmojiSearch() {
        FakeSearchRepo repo;
        NoteService svc(&repo);

        auto results = svc.searchByTitlePaged("😊", 10, 0);
        QVERIFY((int)results.size() >= 1);

        auto contentResults = repo.searchByContent("café");
        QVERIFY((int)contentResults.size() >= 1);
    }

    void longSingleLineContent() {
        FakeSearchRepo repo;
        NoteService svc(&repo);
        auto results = repo.searchByContent("xxxxx");
        // should not crash; expect matches due to long repeated content
        QVERIFY((int)results.size() >= 1);
    }
};

QTEST_MAIN(SearchEdgecasesTests)
#include "search_edgecases_tests.moc"
