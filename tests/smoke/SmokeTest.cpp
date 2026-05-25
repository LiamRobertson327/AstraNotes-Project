#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <vector>
#include <memory>

#include "../src/crypto/EncryptionService.h"
#include "../src/repository/SqliteNoteRepository.h"
#include "../src/model/Note.h"
#include "../src/model/Snapshot.h"

class SmokeTest : public QObject {
    Q_OBJECT

private slots:
    void testEncryptionRoundtrip();
    void testRepositorySaveLoad();
    void testSnapshotAndTrash();
};

void SmokeTest::testEncryptionRoundtrip()
{
    const QString plaintext = "The quick brown fox";
    const QString password = "s3cret";

    auto payload = EncryptionService::encrypt(plaintext, password);
    QVERIFY(!payload.ciphertextBase64.isEmpty());

    auto res = EncryptionService::decrypt(payload.ciphertextBase64,
                                         password,
                                         payload.saltBase64,
                                         payload.ivBase64,
                                         payload.tagBase64);
    QVERIFY(res.success);
    QCOMPARE(res.plaintext, plaintext);
}

void SmokeTest::testRepositorySaveLoad()
{
    QTemporaryDir tmp;
    QVERIFY(tmp.isValid());
    QString dbPath = QDir(tmp.path()).filePath("smoke.db");

    SqliteNoteRepository repo(dbPath);
    QVERIFY(repo.isConnected());

    Note n("general", "Smoke Title");
    n.setContent("Smoke content");

    QVERIFY(repo.save(n));
    qint64 id = n.noteId();
    QVERIFY(id > 0);

    Note* loaded = repo.getById(id);
    QVERIFY(loaded != nullptr);
    QCOMPARE(loaded->title(), n.title());
    QCOMPARE(loaded->content(), n.content());
    delete loaded;
}

void SmokeTest::testSnapshotAndTrash()
{
    QTemporaryDir tmp;
    QString dbPath = QDir(tmp.path()).filePath("smoke2.db");
    SqliteNoteRepository repo(dbPath);
    QVERIFY(repo.isConnected());

    Note n("general", "Snapshot Title");
    n.setContent("Snapshot content");
    QVERIFY(repo.save(n));

    Snapshot s(n.noteId(), n.title(), n.content());
    QVERIFY(repo.saveSnapshot(s));

    QVERIFY(repo.trashNote(n.noteId()));

    auto trashed = repo.getTrashedNotes();
    QVERIFY(!trashed.empty());

    // Purge notes older than 0 days (immediate)
    QVERIFY(repo.purgeTrashedNotes(0));
    auto trashed2 = repo.getTrashedNotes();
    QVERIFY(trashed2.empty());
}

#include "SmokeTest.moc"
QTEST_MAIN(SmokeTest)