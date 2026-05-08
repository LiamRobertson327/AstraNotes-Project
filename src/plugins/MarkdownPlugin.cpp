#include "MarkdownPlugin.h"
#include "MarkdownFormattingPlugin.h"

QList<IFormattingAction*> MarkdownPlugin::getFormattingActions() const {
    QList<IFormattingAction*> actions;
    
    // These are allocated once and should be managed by the caller
    // or converted to shared_ptr in a production system
    actions.append(new BoldAction());
    actions.append(new ItalicAction());
    actions.append(new BulletListAction());
    actions.append(new NumberedListAction());
    actions.append(new HeadingAction(1));
    actions.append(new HeadingAction(2));
    actions.append(new HeadingAction(3));
    actions.append(new CodeBlockAction());
    actions.append(new LinkAction());
    
    return actions;
}
