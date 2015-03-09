#include "GuiWebPlugin.h"
#include "GuiTerminalWindow.h"
#include <QWebFrame>
#include "serialize/QtWebPluginMap.h"

GuiWebPlugin::GuiWebPlugin(GuiMainWindow *p)
    : QToolBar(p),
      mainWnd(p),
      web(this),
      currWebTerm(mainWnd->getCurrentTerminal())
{
    addWidget(&web);

    setMovable(false);
    setAutoFillBackground(true);
    adjustSize();

    connect(&web, SIGNAL(loadFinished(bool)), this, SLOT(on_loadFinished(bool)));

    web.page()->mainFrame()->addToJavaScriptWindowObject("guiwebplugin", this);
    web.page()->mainFrame()->addToJavaScriptWindowObject("guiwebcurrentterm", &currWebTerm);
    web.load(QUrl("qrc:/web/autocomplete.html"));
}

GuiWebPlugin::~GuiWebPlugin()
{

}

QSize GuiWebPlugin::sizeHint() const
{
    return QSize(mainWnd->width()/3, mainWnd->height()/3);
}

void GuiWebPlugin::resizeMe(int w, int h)
{
    this->resize(w, h);
}

void GuiWebPlugin::closeMe()
{
    GuiTerminalWindow *t;

    this->close();
    this->deleteLater();
    mainWnd->webPluginWnd = NULL;

    if (t=mainWnd->getCurrentTerminal()) {
        t->setFocus();
    }
}

void GuiWebPlugin::on_loadFinished(bool ok)
{
    web.setFocus();
    this->move((mainWnd->width()-this->width())/2,
               (mainWnd->height()-this->height())/2);
}

QVariantList GuiWebPlugin::getPersistentList(QString key)
{
    if (qutty_web_plugin_map.hash_map.contains(key))
        return qutty_web_plugin_map.hash_map[key];
    return QVariantList();
}

void GuiWebPlugin::savePersistentList(QString key, QVariantList list)
{
    qutty_web_plugin_map.hash_map[key] = list;
    qutty_web_plugin_map.save();
}


void GuiMainWindow::contextMenuAutoComplete()
{
#ifndef NDEBUG
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
#endif

    if (!this->getCurrentTerminal())
        return;

    if (!webPluginWnd) {
        webPluginWnd = new GuiWebPlugin(this);
        webPluginWnd->show();
        webPluginWnd->setFocus();
    } else {
        webPluginWnd->closeMe();
        webPluginWnd = NULL;
    }
}

