#ifndef MARKDOWNFORMATTINGPLUGIN_H
#define MARKDOWNFORMATTINGPLUGIN_H

#include "IFormattingAction.h"
#include "MarkdownPlugin.h"
#include <QTextEdit>
#include <QList>

class BoldAction : public IFormattingAction {
public:
    QString actionId() const override { return "markdown_bold"; }
    QString actionName() const override { return "Bold"; }
    QString actionToolTip() const override { return "Insert bold text (Ctrl+B)"; }
    QIcon actionIcon() const override { return QIcon(":/icons/bold.png"); }

    void execute(QTextEdit *editor) override {
        if (!editor) return;
        QTextCursor cursor = editor->textCursor();
        QString selected = cursor.selectedText();
        if (!selected.isEmpty()) {
            cursor.insertText("**" + selected + "**");
        } else {
            cursor.insertText("****");
            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, 2);
        }
        editor->setTextCursor(cursor);
    }
};

class ItalicAction : public IFormattingAction {
public:
    QString actionId() const override { return "markdown_italic"; }
    QString actionName() const override { return "Italic"; }
    QString actionToolTip() const override { return "Insert italic text (Ctrl+I)"; }
    QIcon actionIcon() const override { return QIcon(":/icons/italic.png"); }

    void execute(QTextEdit *editor) override {
        if (!editor) return;
        QTextCursor cursor = editor->textCursor();
        QString selected = cursor.selectedText();
        if (!selected.isEmpty()) {
            cursor.insertText("*" + selected + "*");
        } else {
            cursor.insertText("**");
            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, 1);
        }
        editor->setTextCursor(cursor);
    }
};

class BulletListAction : public IFormattingAction {
public:
    QString actionId() const override { return "markdown_bullet_list"; }
    QString actionName() const override { return "Bullet List"; }
    QString actionToolTip() const override { return "Insert bullet point"; }
    QIcon actionIcon() const override { return QIcon(":/icons/bullet_list.png"); }

    void execute(QTextEdit *editor) override {
        if (!editor) return;
        QTextCursor cursor = editor->textCursor();
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.insertText("- ");
        editor->setTextCursor(cursor);
    }
};

class NumberedListAction : public IFormattingAction {
public:
    QString actionId() const override { return "markdown_numbered_list"; }
    QString actionName() const override { return "Numbered List"; }
    QString actionToolTip() const override { return "Insert numbered list"; }
    QIcon actionIcon() const override { return QIcon(":/icons/numbered_list.png"); }

    void execute(QTextEdit *editor) override {
        if (!editor) return;
        QTextCursor cursor = editor->textCursor();
        cursor.movePosition(QTextCursor::StartOfLine);
        cursor.insertText("1. ");
        editor->setTextCursor(cursor);
    }
};

class HeadingAction : public IFormattingAction {
public:
    HeadingAction(int level) : headingLevel(level) {}

    QString actionId() const override { return "markdown_heading_" + QString::number(headingLevel); }
    QString actionName() const override { return "Heading " + QString::number(headingLevel); }
    QString actionToolTip() const override { return "Insert heading level " + QString::number(headingLevel); }
    QIcon actionIcon() const override { return QIcon(":/icons/heading.png"); }

    void execute(QTextEdit *editor) override {
        if (!editor) return;
        QTextCursor cursor = editor->textCursor();
        cursor.movePosition(QTextCursor::StartOfLine);
        QString hashes = QString("#").repeated(headingLevel);
        cursor.insertText(hashes + " ");
        editor->setTextCursor(cursor);
    }

private:
    int headingLevel;
};

class CodeBlockAction : public IFormattingAction {
public:
    QString actionId() const override { return "markdown_code_block"; }
    QString actionName() const override { return "Code Block"; }
    QString actionToolTip() const override { return "Insert code block"; }
    QIcon actionIcon() const override { return QIcon(":/icons/code_block.png"); }

    void execute(QTextEdit *editor) override {
        if (!editor) return;
        QTextCursor cursor = editor->textCursor();
        QString selected = cursor.selectedText();
        if (!selected.isEmpty()) {
            cursor.insertText("```\n" + selected + "\n```");
        } else {
            cursor.insertText("```\n\n```");
            cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, 4);
        }
        editor->setTextCursor(cursor);
    }
};

class LinkAction : public IFormattingAction {
public:
    QString actionId() const override { return "markdown_link"; }
    QString actionName() const override { return "Link"; }
    QString actionToolTip() const override { return "Insert link"; }
    QIcon actionIcon() const override { return QIcon(":/icons/link.png"); }

    void execute(QTextEdit *editor) override {
        if (!editor) return;
        QTextCursor cursor = editor->textCursor();
        QString selected = cursor.selectedText();
        if (!selected.isEmpty()) {
            cursor.insertText("[" + selected + "](url)");
        } else {
            cursor.insertText("[text](url)");
        }
        editor->setTextCursor(cursor);
    }
};

#endif // MARKDOWNFORMATTINGPLUGIN_H
