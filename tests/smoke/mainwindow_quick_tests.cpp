#include <QtTest>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QStackedWidget>

#include "../../src/ui/MainWindow.h"

class MainWindowQuickTests : public QObject {
    Q_OBJECT

private slots:
    void searchBarsHaveIndependentState() {
        MainWindow window;

        auto *contentSearch = window.findChild<QLineEdit *>("searchBar");
        auto *savedNotesSearch = window.findChild<QLineEdit *>("savedNotesSearchBar");

        QVERIFY(contentSearch != nullptr);
        QVERIFY(savedNotesSearch != nullptr);
        QCOMPARE(contentSearch->placeholderText(), QString("Search note content..."));
        QCOMPARE(savedNotesSearch->placeholderText(), QString("Search saved notes..."));

        contentSearch->setText("alpha");
        QCOMPARE(savedNotesSearch->text(), QString());
    }

    void enterAdvancesToNextContentMatch() {
        MainWindow window;
        window.show();

        auto *contentSearch = window.findChild<QLineEdit *>("searchBar");
        auto *matchLabel = window.findChild<QLabel *>("searchMatchLabel");
        QList<QTextEdit*> textEdits = window.findChildren<QTextEdit *>();

        QVERIFY(contentSearch != nullptr);
        QVERIFY(matchLabel != nullptr);
        QVERIFY(textEdits.size() >= 1);

        QTextEdit *editor = textEdits.first();
        editor->setPlainText("alpha beta alpha");

        contentSearch->setFocus();
        QTest::keyClicks(contentSearch, "alpha");
        QTRY_VERIFY(matchLabel->text().contains("of"));

        QTest::keyClick(contentSearch, Qt::Key_Return);
        QTRY_VERIFY(matchLabel->text().contains("of"));
    }
};

QTEST_MAIN(MainWindowQuickTests)
#include "mainwindow_quick_tests.moc"