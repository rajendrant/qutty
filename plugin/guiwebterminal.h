#ifndef GUIWEBTERMINAL
#define GUIWEBTERMINAL

#include "GuiTerminalWindow.h"

class GuiWebTerminal : public QWidget
{
    Q_OBJECT

    GuiTerminalWindow *termWnd;

public:
    GuiWebTerminal(GuiTerminalWindow *p) {
        termWnd = p;
    }
    ~GuiWebTerminal(){}

    Q_INVOKABLE void sendKeys(QString str) {
        wchar_t *buf = new wchar_t[str.length()+1];
        str.toWCharArray(buf);
        luni_send(termWnd->term->ldisc, buf, str.length(), 0);
        delete buf;
    }
};

#endif // GUIWEBTERMINAL
