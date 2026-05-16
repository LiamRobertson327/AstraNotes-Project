#ifndef AUDITLOGGER_H
#define AUDITLOGGER_H

#include <QObject>
#include <QFile>
#include <QMutex>
#include <QAtomicPointer>

class AuditLogger : public QObject {
    Q_OBJECT
public:
    static AuditLogger *instance();
    static void install();

    QString logFilePath() const { return m_logFile.fileName(); }

signals:
    void logAppended(const QString &line);

private:
    explicit AuditLogger(QObject *parent = nullptr);
    ~AuditLogger() override;

    void handleMessage(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);
    static void messageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg);

    QFile m_logFile;
    QMutex m_mutex;

    static QAtomicPointer<AuditLogger> s_instance;
};

#endif // AUDITLOGGER_H
