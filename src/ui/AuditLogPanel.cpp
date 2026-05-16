#include "AuditLogPanel.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QFile>
#include <QTextStream>

AuditLogPanel::AuditLogPanel(QWidget *parent) : QWidget(parent) {
    QVBoxLayout *l = new QVBoxLayout(this);
    table = new QTableWidget(this);
    table->setColumnCount(1);
    table->horizontalHeader()->hide();
    table->horizontalHeader()->setStretchLastSection(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    l->addWidget(table);
}

void AuditLogPanel::loadFromFile(const QString &path) {
    table->setRowCount(0);
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&f);
    int row = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(line));
        ++row;
    }
    table->scrollToBottom();
}

void AuditLogPanel::onNewLogAppended(const QString &line) {
    appendLine(line);
}

void AuditLogPanel::scrollToBottom() {
    table->scrollToBottom();
}

void AuditLogPanel::appendLine(const QString &line) {
    int row = table->rowCount();
    table->insertRow(row);
    table->setItem(row, 0, new QTableWidgetItem(line));
    table->scrollToBottom();
}
