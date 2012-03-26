/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef TERMINALWINDOW_H
#define TERMINALWINDOW_H

#include <QPlainTextEdit>
#include <QFont>
#include <QFontInfo>
#include <QFontMetrics>
#include <QtNetwork/QTcpSocket>
#include <QWidget>
#include "QtCommon.h"
#include "GuiMainWindow.h"
extern "C" {
#include "terminal.h"
#include "putty.h"
}

#define NCFGCOLOURS 22
#define NEXTCOLOURS 240
#define NALLCOLOURS (NCFGCOLOURS + NEXTCOLOURS)

class GuiTerminalWindow : public QWidget
{
    Q_OBJECT
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

    enum {
        BOLD_COLOURS, BOLD_SHADOW, BOLD_FONT
    } bold_mode;

    explicit GuiTerminalWindow(QWidget *parent = 0);
    void keyPressEvent ( QKeyEvent * e );
    void keyReleaseEvent ( QKeyEvent * e );
    void closeEvent(QCloseEvent *closeEvent);
    int from_backend(int is_stderr, const char *data, int len);
    void preDrawTerm();
    void drawTerm();
    void drawText(int row, int col, wchar_t *ch, int len, unsigned long attr, int lattr);

    void setTermFont(FontSpec *f);
    void cfgtopalette(Config *cfg);
    void requestPaste();
    void getClip(wchar_t **p, int *len);
    void writeClip(wchar_t * data, int *attr, int len, int must_deselect);
    void paintText(QPainter &painter, int row, int col, QString str, unsigned long attr);

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
signals:
    
public slots:
    void readyRead ();

};

#endif // TERMINALWINDOW_H
