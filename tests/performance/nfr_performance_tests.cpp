#include <QtTest>
#include <QElapsedTimer>
#include <QTemporaryDir>
#include <QDir>

#include "../../src/model/Note.h"
#include "../../src/repository/SqliteNoteRepository.h"

class NfrPerformanceTests : public QObject {
    Q_OBJECT

private:
    QTemporaryDir m_tempDir;
    std::unique_ptr<SqliteNoteRepository> m_repo;

private slots:
    void initTestCase() {
        QVERIFY(m_tempDir.isValid());

        const QString dbPath = QDir(m_tempDir.path()).filePath("nfr_performance.db");
        m_repo = std::make_unique<SqliteNoteRepository>(dbPath);
        QVERIFY(m_repo != nullptr);
        QVERIFY(m_repo->isConnected());

        for (int index = 0; index < 10000; ++index) {
            Note note("plaintext", QString("Project %1").arg(index, 5, 10, QChar('0')));
            note.setContent(QString("Seed content %1").arg(index));
            QVERIFY2(m_repo->save(note), qPrintable(QString("Failed to seed note %1").arg(index)));
        }
    }

    void saveLatency_under50ms() {
        Note note("plaintext", "Performance Save");
        note.setContent("Timing save operation");

        QElapsedTimer timer;
        timer.start();
        QVERIFY(m_repo->save(note));

        const qint64 elapsedMs = timer.elapsed();
        QVERIFY2(elapsedMs < 50, qPrintable(QString("Save took %1 ms, expected < 50 ms").arg(elapsedMs)));
    }

    void loadLatency_under100ms() {
        Note note("plaintext", "Performance Load");
        note.setContent("Timing load operation");
        QVERIFY(m_repo->save(note));

        QElapsedTimer timer;
        timer.start();
        std::unique_ptr<Note> loaded = m_repo->getById(note.noteId());

        const qint64 elapsedMs = timer.elapsed();
        QVERIFY(loaded != nullptr);
        QVERIFY2(elapsedMs < 100, qPrintable(QString("Load took %1 ms, expected < 100 ms").arg(elapsedMs)));
    }

    void titleSearchLatency_under200ms() {
        (void)m_repo->countTitleMatches(QStringLiteral("Project"));

        QElapsedTimer timer;
        timer.start();
        const int matches = m_repo->countTitleMatches(QStringLiteral("Project"));

        const qint64 elapsedMs = timer.elapsed();
        QCOMPARE(matches, 10000);
        QVERIFY2(elapsedMs < 200, qPrintable(QString("Title search count took %1 ms, expected < 200 ms").arg(elapsedMs)));
    }

    void firstPageLoadLatency_under500ms() {
        (void)m_repo->searchByTitlePaged(QString(), 50, 0);

        QElapsedTimer timer;
        timer.start();
        std::vector<std::unique_ptr<Note>> page = m_repo->searchByTitlePaged(QString(), 50, 0);

        const qint64 elapsedMs = timer.elapsed();
        QCOMPARE(page.size(), static_cast<std::size_t>(50));
        QVERIFY2(elapsedMs < 500, qPrintable(QString("First-page load took %1 ms, expected < 500 ms").arg(elapsedMs)));
    }
};

QTEST_MAIN(NfrPerformanceTests)
#include "nfr_performance_tests.moc"