#include <QtTest>
#include "../../src/model/Note.h"

class TestNote : public Note {
public:
    using Note::Note;
    static int dtorCount;
    ~TestNote() override { ++dtorCount; }
};

int TestNote::dtorCount = 0;

class NoteDeletionLifecycleTests : public QObject {
    Q_OBJECT

private slots:
    void cleanup() { TestNote::dtorCount = 0; }

    void uniquePtrDeletesObject() {
        {
            std::vector<std::unique_ptr<Note>> v;
            v.push_back(std::make_unique<TestNote>("plaintext", "t1"));
            QCOMPARE(TestNote::dtorCount, 0);

            v.pop_back();
            QCOMPARE(TestNote::dtorCount, 1);
        }
        // scope exit should not change (already destroyed)
        QCOMPARE(TestNote::dtorCount, 1);
    }
};

QTEST_MAIN(NoteDeletionLifecycleTests)
#include "note_deletion_lifecycle_tests.moc"
