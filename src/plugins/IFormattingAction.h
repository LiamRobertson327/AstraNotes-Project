#ifndef IFORMATTINGACTION_H
#define IFORMATTINGACTION_H

#include <QString>
#include <QIcon>

class QTextEdit;

class IFormattingAction {
public:
    virtual ~IFormattingAction() = default;

    virtual QString actionId() const = 0;        // e.g., "insert_bold", "insert_list"
    virtual QString actionName() const = 0;      // e.g., "Bold", "Bullet List"
    virtual QString actionToolTip() const = 0;   // Hover text
    virtual QIcon actionIcon() const = 0;        // Icon for the button

    // Called when the action button is clicked
    // editor: The current QTextEdit instance
    virtual void execute(QTextEdit *editor) = 0;
};

#endif // IFORMATTINGACTION_H
