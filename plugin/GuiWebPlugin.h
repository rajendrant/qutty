#ifndef GUIWEBPLUGIN_H
#define GUIWEBPLUGIN_H

#include <QWebView>
class GuiWebPlugin;
#include "GuiMainWindow.h"
#include "GuiWebTerminal.h"

class GuiWebPlugin : public QToolBar
{
    Q_OBJECT

    GuiMainWindow *mainWnd;
    QWebView web;
    GuiWebTerminal currWebTerm;

public:
    GuiWebPlugin(GuiMainWindow *p);
    ~GuiWebPlugin();
    QSize sizeHint() const;

    Q_INVOKABLE void resizeMe(int w, int h);
    Q_INVOKABLE void closeMe();
    Q_INVOKABLE QVariantList getPersistentList(QString key);
    Q_INVOKABLE void savePersistentList(QString key, QVariantList list);

public slots:
    void on_loadFinished(bool ok);
};

#endif // GUIWEBPLUGIN_H
