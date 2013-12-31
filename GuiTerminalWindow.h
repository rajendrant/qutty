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
#include <QElapsedTimer>
#include "QtCommon.h"
#include "tmux/tmux.h"
#include "tmux/TmuxGateway.h"
#include "tmux/TmuxWindowPane.h"
#include "GuiDrag.h"
#include "GuiBase.h"
extern "C" {
#include "terminal.h"
#include "putty.h"
}

#define NCFGCOLOURS 22
#define NEXTCOLOURS 240
#define NALLCOLOURS (NCFGCOLOURS + NEXTCOLOURS)

class GuiMainWindow;

class GuiTerminalWindow : public QAbstractScrollArea, public GuiBase
{
    Q_OBJECT
    Q_INTERFACES(GuiBase)

private:
    enum tmux_mode_t _tmuxMode;
    TmuxGateway *_tmuxGateway;
    GuiMainWindow *mainWindow;

    void showContextMenu(QMouseEvent *e);

    QFont _font;
    int fontWidth, fontHeight, fontAscent;
    struct unicode_data ucsdata;
    Actual_Socket as;
    QTcpSocket *qtsock;
    bool _any_update;
    QRegion termrgn;
    QColor colours[NALLCOLOURS];

    // to detect mouse double/triple clicks
    Mouse_Action mouseButtonAction;
    QElapsedTimer mouseClickTimer;


    enum {
        BOLD_COLOURS, BOLD_SHADOW, BOLD_FONT
    } bold_mode;

    // members for drag-drop support
    QPoint dragStartPos;

    // support for clipboard paste
    wchar_t *clipboard_contents;
    int clipboard_length;

    // session title
    QString runtime_title;  // given by terminal/shell
    QString custom_title;   // given by user
    QString temp_title;

public:
    Config cfg;
    Terminal *term;
    Backend *backend;
    void *backhandle;
    void *ldisc;

    bool userClosingTab;
    bool isSockDisconnected;

    // order-of-usage
    uint32_t mru_count;

    explicit GuiTerminalWindow(QWidget *parent, GuiMainWindow *mainWindow);
    virtual ~GuiTerminalWindow();

    GuiMainWindow *getMainWindow() {
        return mainWindow;
    }

    /*
     * follow a two phased construction
     * 1. GuiMainWindow::newTerminal()  -> create
     * 2. termWnd->initTerminal() -> init the internals with config
     */
    int initTerminal();
    int restartTerminal();
    int reconfigureTerminal(Config *new_cfg);

    void createSplitLayout(GuiBase::SplitType split, GuiTerminalWindow *newTerm);

    void keyPressEvent ( QKeyEvent * e );
    void keyReleaseEvent ( QKeyEvent * e );
    int from_backend(int is_stderr, const char *data, size_t len);
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

    void closeTerminal();
    void reqCloseTerminal(bool userConfirm);

    int getFontWidth() { return fontWidth; };
    int getFontHeight() { return fontHeight; };

    QWidget *getWidget() { return this; }

    // Needed functions for drag-drop support
    void dragStartEvent (QMouseEvent *e);
    void dragEnterEvent (QDragEnterEvent *e);
    void dragLeaveEvent (QDragLeaveEvent *e);
    void dragMoveEvent (QDragMoveEvent *e);
    void dropEvent (QDropEvent *e);

    void populateAllTerminals(vector<GuiTerminalWindow*> *list) {
        list->push_back(this);
    }

    QString getSessionTitle() { return runtime_title; }
    QString getCustomSessionTitle() { return custom_title; }
    void setSessionTitle(QString t) {
        if (runtime_title != t) {
            runtime_title = t;
            on_sessionTitleChange();
        }
    }
    void setCustomSessionTitle(QString t) {
        custom_title = t;
        on_sessionTitleChange();
    }

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
    void on_sessionTitleChange(bool force=false);

};

#endif // TERMINALWINDOW_H
