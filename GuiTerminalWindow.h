/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef TERMINALWINDOW_H
#define TERMINALWINDOW_H

#include <QFont>
#include <QFontInfo>
#include <QFontMetrics>
#include <QtNetwork/QTcpSocket>
#include <QAbstractScrollArea>
#include "QtCommon.h"
#include "GuiMainWindow.h"
#include <QElapsedTimer>
#include "tmux/tmux.h"
#include "tmux/TmuxGateway.h"
#include "tmux/TmuxWindowPane.h"
extern "C" {
#include "terminal.h"
#include "putty.h"
}

#define NCFGCOLOURS 22
#define NEXTCOLOURS 240
#define NALLCOLOURS (NCFGCOLOURS + NEXTCOLOURS)

class GuiTerminalWindow : public QAbstractScrollArea
{
    Q_OBJECT
private:
    enum tmux_mode_t _tmuxMode;
    TmuxGateway *_tmuxGateway;
    GuiMainWindow *mainWindow;

public:
    QFont *_font;
    QFontMetrics *_fontMetrics;
    int fontWidth, fontHeight;
    void *ldisc;
    Terminal *term;
    Backend *backend;
    void *backhandle;
    struct unicode_data ucsdata;
    Actual_Socket as;
    QTcpSocket *qtsock;
    Config cfg;
    bool _any_update;
    QRegion termrgn;
    QColor colours[NALLCOLOURS];

    // to detect mouse double/triple clicks
    Mouse_Action mouseButtonAction;
    QElapsedTimer mouseClickTimer;

    bool userClosingTab;

    enum {
        BOLD_COLOURS, BOLD_SHADOW, BOLD_FONT
    } bold_mode;

    explicit GuiTerminalWindow(QWidget *parent, GuiMainWindow *mainWindow);
    ~GuiTerminalWindow();

    GuiMainWindow *getMainWindow() {
        return mainWindow;
    }

    /*
     * follow a two phased construction
     * 1. GuiMainWindow::newTerminal()  -> create
     * 2. termWnd->initTerminal() -> init the internals with config
     */
    int initTerminal();

    void keyPressEvent ( QKeyEvent * e );
    void keyReleaseEvent ( QKeyEvent * e );
    int from_backend(int is_stderr, const char *data, int len);
    void preDrawTerm();
    void drawTerm();
    void drawText(int row, int col, wchar_t *ch, int len, unsigned long attr, int lattr);

    void setTermFont(Config *cfg);
    void cfgtopalette(Config *cfg);
    void requestPaste();
    void getClip(wchar_t **p, int *len);
    void writeClip(wchar_t * data, int *attr, int len, int must_deselect);
    void paintText(QPainter &painter, int row, int col,
                   const QString &str, unsigned long attr);
    void paintCursor(QPainter &painter, int row, int col,
                     const QString &str, unsigned long attr);

    void setScrollBar(int total, int start, int page);
    int TranslateKey(QKeyEvent *keyevent, char *output);

    int initTmuxControllerMode(char *tmux_version);
    TmuxWindowPane *initTmuxClientTerminal(TmuxGateway *gateway, int id, int width, int height);
    void startDetachTmuxControllerMode();
    TmuxGateway *tmuxGateway() { return _tmuxGateway; }

protected:
    void paintEvent ( QPaintEvent * e );
    void 	mouseDoubleClickEvent ( QMouseEvent * e );
    void 	mouseMoveEvent ( QMouseEvent * e );
    void 	mousePressEvent ( QMouseEvent * e );
    void 	mouseReleaseEvent ( QMouseEvent * e );
    void 	resizeEvent ( QResizeEvent * e );
    bool event(QEvent *event);
    void focusInEvent ( QFocusEvent * e );
    void focusOutEvent ( QFocusEvent * e );

public slots:
    void readyRead ();
    void vertScrollBarAction(int action);
    void vertScrollBarMoved(int value);
    void detachTmuxControllerMode();
    void sockError(QAbstractSocket::SocketError socketError);
    void sockDisconnected();

};

#endif // TERMINALWINDOW_H
