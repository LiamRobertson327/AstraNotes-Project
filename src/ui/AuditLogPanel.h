#ifndef AUDITLOGPANEL_H
#define AUDITLOGPANEL_H

#include <QWidget>
#include <QTableWidget>

class AuditLogPanel : public QWidget {
    Q_OBJECT
public:
    explicit AuditLogPanel(QWidget *parent = nullptr);
    void loadFromFile(const QString &path);
    void scrollToBottom();

public slots:
    void onNewLogAppended(const QString &line);

private:
    QTableWidget *table;
    void appendLine(const QString &line);
};

#endif // AUDITLOGPANEL_H
