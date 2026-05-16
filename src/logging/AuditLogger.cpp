#include "AuditLogger.h"
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QTextStream>
#include <QCoreApplication>
#include <QRegularExpression>

static qint64 extractNoteId(const QString &message) {
    const QRegularExpression noteIdPattern(R"((?:note\s+id|snapshot\s+id|id)\s*[:#-]?\s*(\d+))", QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = noteIdPattern.match(message);
    if (!match.hasMatch()) {
        return -1;
    }
    bool ok = false;
    const qint64 noteId = match.captured(1).toLongLong(&ok);
    return ok ? noteId : -1;
}

QAtomicPointer<AuditLogger> AuditLogger::s_instance = nullptr;

AuditLogger::AuditLogger(QObject *parent) : QObject(parent) {
    // Determine a safe writable log location
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dataDir.isEmpty()) {
        dataDir = QCoreApplication::applicationDirPath();
    }
    QDir dir(dataDir);
    if (!dir.exists()) dir.mkpath(".");
    QString logPath = dir.filePath("audit.log");
    m_logFile.setFileName(logPath);
    if (!m_logFile.open(QIODevice::Append | QIODevice::Text)) {
        // best-effort: if we can't open, fall back to no-op
    }
}

AuditLogger::~AuditLogger() {
    if (m_logFile.isOpen()) m_logFile.close();
}

AuditLogger *AuditLogger::instance() {
    AuditLogger *inst = s_instance.loadAcquire();
    if (!inst) {
        AuditLogger *expected = nullptr;
        AuditLogger *created = new AuditLogger(nullptr);
        if (s_instance.testAndSetOrdered(expected, created)) {
            inst = created;
        } else {
            delete created;
            inst = s_instance.loadAcquire();
        }
    }
    return inst;
}

void AuditLogger::install() {
    // ensure instance created
    AuditLogger::instance();
    qInstallMessageHandler(AuditLogger::messageHandler);
}

void AuditLogger::messageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg) {
    AuditLogger *logger = AuditLogger::instance();
    if (logger) {
        logger->handleMessage(type, ctx, msg);
    }
}

void AuditLogger::handleMessage(QtMsgType type, const QMessageLogContext &ctx, const QString &msg) {
    QMutexLocker lock(&m_mutex);
    QString severity;
    switch (type) {
        case QtDebugMsg: severity = "debug"; break;
        case QtInfoMsg: severity = "info"; break;
        case QtWarningMsg: severity = "warning"; break;
        case QtCriticalMsg: severity = "error"; break;
        case QtFatalMsg: severity = "fatal"; break;
        default: severity = "log"; break;
    }

    const qint64 noteId = extractNoteId(msg);

    QString action = msg.trimmed();
    action.remove(QRegularExpression(R"(^\[[^\]]+\]\s*)"));
    action.remove(QRegularExpression(R"(\s*\([^\)]*\)\s*$)"));
    action = action.simplified().toLower();

    if (action.contains("auto-save triggered") || action.contains("autosave")) {
        action = "note autosaved";
    } else if (action.contains("saving note") || action.contains("save successful") || action.contains("note saved")) {
        action = "note saved";
    } else if (action.contains("created new note") || action.contains("new note")) {
        action = "note created";
    } else if (action.contains("loading note with id") || action.contains("loaded note")) {
        action = "note opened";
    } else if (action.contains("moved note to trash") || action.contains("trashed note")) {
        action = "note trashed";
    } else if (action.contains("restored note from trash") || action.contains("note restored")) {
        action = "note restored";
    } else if (action.contains("purged trashed notes") || action.contains("trash purged")) {
        action = "trash purged";
    } else if (action.contains("snapshot created") || action.contains("snapshot saved")) {
        action = "snapshot saved";
    } else if (action.contains("reverted to snapshot") || action.contains("snapshot restored")) {
        action = "snapshot restored";
    } else if (action.contains("deleted snapshot") || action.contains("snapshot deleted")) {
        action = "snapshot deleted";
    } else if (action.contains("encrypting note") || action.contains("encrypted note")) {
        action = "note encrypted";
    } else if (action.contains("decrypting note") || action.contains("decrypted note")) {
        action = "note decrypted";
    } else if (action.contains("saved settings") || action.contains("updated retention") || action.contains("settings updated")) {
        action = "settings updated";
    } else if (action.contains("search notes") || action.contains("search match") || action.contains("search performed")) {
        action = "search performed";
    } else if (action.contains("qapplication created") || action.contains("mainwindow shown") || action.contains("app started")) {
        action = "app started";
    }

    if (noteId >= 0 && (action.contains("note") || action.contains("snapshot") || action.contains("trash") || action.contains("encrypt") || action.contains("decrypt"))) {
        action += QString(" #%1").arg(noteId);
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString line = QString("%1 - %2 - %3").arg(timestamp, severity, action);

    if (m_logFile.isOpen()) {
        QTextStream out(&m_logFile);
        out << line << "\n";
        out.flush();
    }

    emit logAppended(line);

    if (type == QtFatalMsg) {
        abort();
    }
}
