/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include "QtLogDbg.h"
#include <QDebug>
#include <QApplication>
#include <QPainter>
#include <QClipboard>
#include <QScrollBar>
#include <QMessageBox>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiSplitter.h"
#include "GuiMenu.h"
#include "GuiTabWidget.h"

GuiTerminalWindow::GuiTerminalWindow(QWidget *parent, GuiMainWindow *mainWindow) :
    QAbstractScrollArea(parent),
    clipboard_contents(NULL),
    clipboard_length(0),
    runtime_title(""),
    custom_title(""),
    mru_count(0)
{
    this->mainWindow = mainWindow;

    setFrameShape(QFrame::NoFrame);
    setWindowState(Qt::WindowMaximized);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(verticalScrollBar(), SIGNAL(actionTriggered(int)), this, SLOT(vertScrollBarAction(int)));
    connect(verticalScrollBar(), SIGNAL(sliderMoved(int)), this, SLOT(vertScrollBarMoved(int)));
    connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(vertScrollBarMoved(int)));

    setFocusPolicy(Qt::StrongFocus);
    _any_update = false;

    termrgn = QRegion();
    term = NULL;
    ldisc = NULL;
    backend = NULL;
    backhandle = NULL;
    qtsock = NULL;
    userClosingTab = false;
    isSockDisconnected = false;

    mouseButtonAction = MA_NOTHING;
    setMouseTracking(true);
    viewport()->setCursor(Qt::IBeamCursor);

    _tmuxMode = TMUX_MODE_NONE;
    _tmuxGateway = NULL;

    // enable drag-drop
    setAcceptDrops(true);
}

GuiTerminalWindow::~GuiTerminalWindow()
{
    if (_tmuxMode==TMUX_MODE_GATEWAY && _tmuxGateway) {
        _tmuxGateway->initiateDetach();
        delete _tmuxGateway;
    }

    if (ldisc) {
        ldisc_free(ldisc);
        ldisc = NULL;
    }
    if (backend) {
        backend->free(backhandle);
        backhandle = NULL;
        backend = NULL;
        term_provide_resize_fn(term, NULL, NULL);
        term_free(term);
        qtsock->close();
        term = NULL;
        qtsock = NULL;
    }
}

extern "C" Socket get_ssh_socket(void *handle);
extern "C" Socket get_telnet_socket(void *handle);

int GuiTerminalWindow::initTerminal()
{
    char *realhost = NULL;
    char *ip_addr = cfg.host;
    void *logctx;

    set_title(this, cfg.host);

    memset(&ucsdata, 0, sizeof(struct unicode_data));
    init_ucs(&cfg, &ucsdata);
    setTermFont(&cfg);
    cfgtopalette(&cfg);

    backend = backend_from_proto(cfg.protocol);
    const char * error = backend->init(this, &backhandle, &cfg, (char*)ip_addr, cfg.port, &realhost, 1, 0);
    if (realhost)
        sfree(realhost);

    if (error) {
        char msg[512];
        _snprintf(msg, sizeof(msg),
                    "Unable to open connection to\n"
                    "%.800s\n" "%s", cfg_dest(&cfg), error);
        qt_message_box(this, APPNAME " Error", msg);
        backend = NULL;
        term = NULL;
        ldisc = NULL;
        goto cu0;
    }

    term = term_init(&cfg, &ucsdata, this);
    logctx = log_init(NULL, &cfg);
    term_provide_logctx(term, logctx);

    term_size(term,
              this->viewport()->height()/fontHeight,
              this->viewport()->width()/fontWidth, cfg.savelines);

    switch(cfg.protocol) {
    case PROT_TELNET:
        as = (Actual_Socket)get_telnet_socket(backhandle);
        break;
    case PROT_SSH:
        as = (Actual_Socket)get_ssh_socket(backhandle);
        break;
    default:
        assert(0);
    }
    qtsock = as->qtsock;
    QObject::connect(as->qtsock, SIGNAL(readyRead()), this, SLOT(readyRead()));
    QObject::connect(as->qtsock, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(sockError(QAbstractSocket::SocketError)));
    QObject::connect(as->qtsock, SIGNAL(disconnected()),
                     this, SLOT(sockDisconnected()));

    /*
     * Connect the terminal to the backend for resize purposes.
     */
    term_provide_resize_fn(term, backend->size, backhandle);

    /*
     * Set up a line discipline.
     */
    ldisc = ldisc_create(&cfg, term, backend, backhandle, this);

    return 0;

cu0:
    return -1;
}

int GuiTerminalWindow::restartTerminal()
{
    if (_tmuxMode==TMUX_MODE_GATEWAY && _tmuxGateway) {
        _tmuxGateway->initiateDetach();
        delete _tmuxGateway;
    }

    if (ldisc) {
        ldisc_free(ldisc);
        ldisc = NULL;
    }
    if (backend) {
        backend->free(backhandle);
        backhandle = NULL;
        backend = NULL;
        term_provide_resize_fn(term, NULL, NULL);
        term_free(term);
        qtsock->close();
        term = NULL;
        qtsock = NULL;
    }
    isSockDisconnected = false;
    return initTerminal();
}

int GuiTerminalWindow::reconfigureTerminal(Config *new_cfg)
{
    Config prev_cfg = this->cfg;

    this->cfg = *new_cfg;

    /* Pass new config data to the logging module */
    log_reconfig(term->logctx, &cfg);

    /*
     * Flush the line discipline's edit buffer in the
     * case where local editing has just been disabled.
     */
    if (ldisc)
        ldisc_send(ldisc, NULL, 0, 0);

    cfgtopalette(&cfg);

    /* Pass new config data to the terminal */
    term_reconfig(term, &cfg);

    /* Pass new config data to the back end */
    if (backend)
        backend->reconfig(backhandle, &cfg);

    /* Screen size changed ? */
    if (cfg.height != prev_cfg.height ||
        cfg.width != prev_cfg.width ||
        cfg.savelines != prev_cfg.savelines ||
        cfg.resize_action == RESIZE_FONT ||
        (cfg.resize_action == RESIZE_EITHER /*&& IsZoomed(hwnd)*/) ||
        cfg.resize_action == RESIZE_DISABLED)
        term_size(term, cfg.height, cfg.width, cfg.savelines);

    if (cfg.alwaysontop != prev_cfg.alwaysontop) {
    }

    if (strcmp(cfg.font.name, prev_cfg.font.name) != 0 ||
        strcmp(cfg.line_codepage, prev_cfg.line_codepage) != 0 ||
        cfg.font.isbold != prev_cfg.font.isbold ||
        cfg.font.height != prev_cfg.font.height ||
        cfg.font.charset != prev_cfg.font.charset ||
        cfg.font_quality != prev_cfg.font_quality ||
        cfg.vtmode != prev_cfg.vtmode ||
        cfg.bold_colour != prev_cfg.bold_colour ||
        cfg.resize_action == RESIZE_DISABLED ||
        cfg.resize_action == RESIZE_EITHER ||
        (cfg.resize_action != prev_cfg.resize_action)) {
        init_ucs(&cfg, &ucsdata);
        setTermFont(&cfg);
    }

    repaint();

    return 0;
}

TmuxWindowPane *GuiTerminalWindow::initTmuxClientTerminal(TmuxGateway *gateway,
                                        int id, int width, int height)
{
    TmuxWindowPane *tmuxPane = NULL;

    memset(&ucsdata, 0, sizeof(struct unicode_data));
    init_ucs(&cfg, &ucsdata);
    setTermFont(&cfg);
    cfgtopalette(&cfg);

    term = term_init(&cfg, &ucsdata, this);
    void *logctx = log_init(NULL, &cfg);
    term_provide_logctx(term, logctx);
    // resize according to config if window is smaller
    if ( !(mainWindow->windowState() & Qt::WindowMaximized) &&
          ( mainWindow->size().width() < cfg.width*fontWidth ||
            mainWindow->size().height() < cfg.height*fontHeight)) {
        mainWindow->resize(cfg.width*fontWidth,
                           cfg.height*fontHeight);
    }
    term_size(term, height, width, cfg.savelines);

    _tmuxMode = TMUX_MODE_CLIENT;
    _tmuxGateway = gateway;
    cfg.protocol = PROT_TMUX_CLIENT;
    cfg.port = -1;
    cfg.width = width;
    cfg.height = height;

    backend = backend_from_proto(cfg.protocol);
    // HACK - pass paneid in port
    backend->init(this, &backhandle, &cfg, NULL, id, NULL, 0, 0);
    tmuxPane = new TmuxWindowPane(gateway, this);
    tmuxPane->id = id;
    tmuxPane->width = width;
    tmuxPane->height = height;

    as = NULL;
    qtsock = NULL;

    /*
     * Connect the terminal to the backend for resize purposes.
     */
    term_provide_resize_fn(term, backend->size, backhandle);

    /*
     * Set up a line discipline.
     */
    ldisc = ldisc_create(&cfg, term, backend, backhandle, this);
    return tmuxPane;
}

void GuiTerminalWindow::keyPressEvent ( QKeyEvent *e )
{
    noise_ultralight(e->key());
    if (!term) return;

    // skip ALT SHIFT CTRL keypress events
    switch(e->key()) {
    case Qt::Key_Alt:
    case Qt::Key_AltGr:
    case Qt::Key_Control:
    case Qt::Key_Shift:
        return;
    }

    char buf[16];
    int len = TranslateKey(e, buf);
    assert(len<16);
    if (len>0 || len==-2) {
        term_nopaste(term);
        term_seen_key_event(term);
        ldisc_send(ldisc, buf, len, 1);
        //show_mouseptr(0);
    } else if(len==-1) {
        wchar_t bufwchar[16];
        len = 0;
        // Treat Alt by just inserting an Esc before everything else
        if (!(e->modifiers() & Qt::ControlModifier) && e->modifiers() & Qt::AltModifier){
            /*
             * In PuTTY, Left and right Alt will act differently
             * Ex: In english keybord layout, Left Alt + W = "" / Right Alt + W = "W"
             * In QuTTY, Left and left Alt will act same
             * Ex: In english keybord layout, Left Alt + W = "" / Right Alt + W = ""
             */
            bufwchar[len++] = 0x1b;
        }
        len += e->text().toWCharArray(bufwchar+len);
        if (len>0 && len<16) {
            term_nopaste(term);
            term_seen_key_event(term);
            luni_send(ldisc, bufwchar, len, 1);
        }
    }
}

void GuiTerminalWindow::keyReleaseEvent ( QKeyEvent * e )
{
    noise_ultralight(e->key());
}

void GuiTerminalWindow::readyRead ()
{
    char buf[20480];
    int len = qtsock->read(buf, sizeof(buf));
    noise_ultralight(len);
    (*as->plug)->receive(as->plug, 0, buf, len);

    if(qtsock->bytesAvailable()>0) readyRead();
}

void GuiTerminalWindow::paintEvent (QPaintEvent *e)
{
    QPainter painter(viewport());

    if(!term)
        return;

    painter.fillRect(e->rect(), colours[258]);

    for(int i=0; i<e->region().rects().size(); i++) {
        const QRect &r = e->region().rects().at(i);
        int row = r.top()/fontHeight;
        int colstart = r.left()/fontWidth;
        int rowend = (r.bottom()+1)/fontHeight;
        int colend = (r.right()+1)/fontWidth;
        for(; row<rowend && row<term->rows; row++) {
            for(int col=colstart; col<colend && col<term->cols; ) {
                uint attr = term->dispstr_attr[row*term->cols + col];
                int coldiff = col+1;
                for(;attr==term->dispstr_attr[row*term->cols + coldiff];
                    coldiff++);
                if (coldiff >= term->cols)
                    coldiff = term->cols;
                QString str = QString::fromWCharArray(&term->dispstr[row*term->cols + col], coldiff-col);
                paintText(painter, row, col, str, attr);

                // paint cursor
                if (attr & (TATTR_ACTCURS | TATTR_PASCURS))
                    paintCursor(painter, row, col, str, attr);
                col = coldiff;
            }
        }
    }
}

void GuiTerminalWindow::paintText(QPainter &painter, int row, int col,
                                  const QString &str, unsigned long attr)
{
    if ((attr & TATTR_ACTCURS) && (cfg.cursor_type == 0 || term->big_cursor)) {
    attr &= ~(ATTR_REVERSE|ATTR_BLINK|ATTR_COLOURS);
    if (bold_mode == BOLD_COLOURS)
        attr &= ~ATTR_BOLD;

    /* cursor fg and bg */
    attr |= (260 << ATTR_FGSHIFT) | (261 << ATTR_BGSHIFT);
    }

    int nfg = ((attr & ATTR_FGMASK) >> ATTR_FGSHIFT);
    int nbg = ((attr & ATTR_BGMASK) >> ATTR_BGSHIFT);
    if (attr & ATTR_REVERSE) {
        int t = nfg;
        nfg = nbg;
        nbg = t;
    }
    if (bold_mode == BOLD_COLOURS && (attr & ATTR_BOLD)) {
        if (nfg < 16) nfg |= 8;
        else if (nfg >= 256) nfg |= 1;
    }
    if (bold_mode == BOLD_COLOURS && (attr & ATTR_BLINK)) {
        if (nbg < 16) nbg |= 8;
        else if (nbg >= 256) nbg |= 1;
    }
    painter.fillRect(QRect(col*fontWidth, row*fontHeight,
                          fontWidth*str.length(), fontHeight),
                          colours[nbg]);
    painter.setPen(colours[nfg]);
    painter.drawText(col*fontWidth,
                     row*fontHeight+fontAscent,
                     str);
}

void GuiTerminalWindow::paintCursor(QPainter &painter, int row, int col,
                                  const QString &str, unsigned long attr)
{
    int fnt_width;
    int char_width;
    int ctype = cfg.cursor_type;

    if ((attr & TATTR_ACTCURS) && (ctype == 0 || term->big_cursor)) {
        return paintText(painter, row, col, str, attr);
    }

    fnt_width = char_width = fontWidth;
    if (attr & ATTR_WIDE)
    char_width *= 2;
    int x = col*fnt_width;
    int y = row*fontHeight;

    if ((attr & TATTR_PASCURS) && (ctype == 0 || term->big_cursor)) {
        QPoint points[] = {
            QPoint(x, y),
            QPoint(x, y+fontHeight-1),
            QPoint(x+char_width-1, y+fontHeight-1),
            QPoint(x+char_width-1, y)
        };
        painter.setPen(colours[261]);
        painter.drawPolygon(points, 4);
    } else if ((attr & (TATTR_ACTCURS | TATTR_PASCURS)) && ctype != 0) {
        int startx, starty, dx, dy, length, i;
        if (ctype == 1) {
            startx = x;
            starty = y + fontMetrics().descent();
            dx = 1;
            dy = 0;
            length = char_width;
        } else {
            int xadjust = 0;
            if (attr & TATTR_RIGHTCURS)
            xadjust = char_width - 1;
            startx = x + xadjust;
            starty = y;
            dx = 0;
            dy = 1;
            length = fontHeight;
        }
        if (attr & TATTR_ACTCURS) {
            // To draw the vertical and underline active cursors
            painter.setPen(colours[261]);
            painter.drawLine(startx, starty + length * dx, startx + length * dx, starty + length);
        } else {
            // To draw the vertical and underline passive cursors
            painter.setPen(colours[261]);
            for (i = 0; i < length; i++) {
                if (i % 2 == 0) {
                    painter.drawPoint(startx, starty + length * dx);
                }
                startx += dx;
                starty += dy;
            }
        }
    }
}

int GuiTerminalWindow::from_backend(int is_stderr, const char *data, int len)
{
    if (_tmuxMode==TMUX_MODE_GATEWAY && _tmuxGateway) {
        int rc = _tmuxGateway->fromBackend(is_stderr, data, len);
        if (rc) {
            if (rc >= 0 && rc < len && _tmuxMode == TMUX_MODE_GATEWAY_DETACH_INIT) {
                detachTmuxControllerMode();
                return term_data(term, is_stderr, data+rc, len-rc);
            }
        }
    }
    return term_data(term, is_stderr, data, len);
}

void GuiTerminalWindow::preDrawTerm()
{
    termrgn = QRegion();
}

void GuiTerminalWindow::drawTerm()
{
    this->viewport()->update(termrgn);
}

void GuiTerminalWindow::drawText(int row, int col, wchar_t * /*ch*/, int len, unsigned long attr, int /*lattr*/)
{
    if (attr & TATTR_COMBINING) {
        // TODO NOT_YET_IMPLEMENTED
        return;
    }
    termrgn |= QRect(col*fontWidth, row*fontHeight, fontWidth*len, fontHeight);
}

void GuiTerminalWindow::setTermFont(Config *cfg)
{
    _font.setFamily(cfg->font.name);
    _font.setPointSize(cfg->font.height);
    _font.setStyleHint(QFont::TypeWriter);

    if (cfg->font_quality == FQ_NONANTIALIASED)
        _font.setStyleStrategy(QFont::NoAntialias);
    else if (cfg->font_quality == FQ_ANTIALIASED)
        _font.setStyleStrategy(QFont::PreferAntialias);
    setFont(_font);

    QFontMetrics fontMetrics = QFontMetrics(_font);
    fontWidth = fontMetrics.width(QChar('a'));
    fontHeight = fontMetrics.height();
    fontAscent = fontMetrics.ascent();
}

void GuiTerminalWindow::cfgtopalette(Config *cfg)
{
    int i;
    static const int ww[] = {
    256, 257, 258, 259, 260, 261,
    0, 8, 1, 9, 2, 10, 3, 11,
    4, 12, 5, 13, 6, 14, 7, 15
    };

    for (i = 0; i < 22; i++) {
        colours[ww[i]] = QColor::fromRgb(cfg->colours[i][0], cfg->colours[i][1], cfg->colours[i][2]);
    }
    for (i = 0; i < NEXTCOLOURS; i++) {
    if (i < 216) {
        int r = i / 36, g = (i / 6) % 6, b = i % 6;
        r = r ? r * 40 + 55 : 0;
        g = g ? g * 40 + 55 : 0;
        b = b ? b * 40 + 55 : 0;
        colours[i+16] = QColor::fromRgb(r, g, b);
    } else {
        int shade = i - 216;
        shade = shade * 10 + 8;
        colours[i+16] = QColor::fromRgb(shade, shade, shade);
    }
    }

    /* Override with system colours if appropriate * /
    if (cfg.system_colour)
        systopalette();*/
}

/*
 * Translate a raw mouse button designation (LEFT, MIDDLE, RIGHT)
 * into a cooked one (SELECT, EXTEND, PASTE).
 */
static Mouse_Button translate_button(Config *cfg, Mouse_Button button)
{
    if (button == MBT_LEFT)
    return MBT_SELECT;
    if (button == MBT_MIDDLE)
    return cfg->mouse_is_xterm == 1 ? MBT_PASTE : MBT_EXTEND;
    if (button == MBT_RIGHT)
    return cfg->mouse_is_xterm == 1 ? MBT_EXTEND : MBT_PASTE;
    assert(0);
    return MBT_NOTHING;			       /* shouldn't happen */
}

void 	GuiTerminalWindow::mouseDoubleClickEvent ( QMouseEvent * e )
{
    noise_ultralight(e->x()<<16 | e->y());
    if (!term) return;

    if(e->button()==Qt::RightButton &&
            ((e->modifiers() & Qt::ControlModifier) || (cfg.mouse_is_xterm == 2))) {
        // TODO right click menu
    }
    Mouse_Button button, bcooked;
    button = e->button()==Qt::LeftButton ? MBT_LEFT :
             e->button()==Qt::RightButton ? MBT_RIGHT :
             e->button()==Qt::MidButton ? MBT_MIDDLE : MBT_NOTHING;
    // assert(button!=MBT_NOTHING);
    if(button == MBT_NOTHING)
        return;
    int x = e->x()/fontWidth, y = e->y()/fontHeight, mod=e->modifiers();
    bcooked = translate_button(&cfg, button);

    // detect single/double/triple click
    mouseClickTimer.start();
    mouseButtonAction = MA_2CLK;

    qDebug()<<__FUNCTION__<<x<<y<<mod<<button<<bcooked<<mouseButtonAction;
    term_mouse(term, button, bcooked, mouseButtonAction,
               x,y, mod&Qt::ShiftModifier, mod&Qt::ControlModifier, mod&Qt::AltModifier);
    e->accept();
}

//#define (e) e->button()&Qt::LeftButton
void 	GuiTerminalWindow::mouseMoveEvent ( QMouseEvent * e )
{
    noise_ultralight(e->x()<<16 | e->y());
    if (e->buttons() == Qt::NoButton) {
        mainWindow->toolBarTerminalTop.processMouseMoveTerminalTop(this, e);
        return;
    }
    if (!term) return;

    if (e->buttons() == Qt::LeftButton &&
        ((e->modifiers() & Qt::ControlModifier) || (cfg.mouse_is_xterm == 2)) &&
        (e->pos() - dragStartPos).manhattanLength() >= QApplication::startDragDistance()) {
        // start of drag
        this->dragStartEvent(e);
        return;
    }

    Mouse_Button button, bcooked;
    button = e->buttons()&Qt::LeftButton ? MBT_LEFT :
             e->buttons()&Qt::RightButton ? MBT_RIGHT :
             e->buttons()&Qt::MidButton ? MBT_MIDDLE : MBT_NOTHING;
    //assert(button!=MBT_NOTHING);
    if(button == MBT_NOTHING)
        return;
    int x = e->x()/fontWidth, y = e->y()/fontHeight, mod=e->modifiers();
    bcooked = translate_button(&cfg, button);
    term_mouse(term, button, bcooked, MA_DRAG,
               x,y, mod&Qt::ShiftModifier, mod&Qt::ControlModifier, mod&Qt::AltModifier);
    e->accept();
}

// Qt 5.0 supports qApp->styleHints()->mouseDoubleClickInterval()
#define CFG_MOUSE_TRIPLE_CLICK_INTERVAL 500

void 	GuiTerminalWindow::mousePressEvent ( QMouseEvent * e )
{
    noise_ultralight(e->x()<<16 | e->y());
    if (!term) return;

    if(e->button()==Qt::LeftButton &&
            ((e->modifiers() & Qt::ControlModifier) || (cfg.mouse_is_xterm == 2))) {
        // possible start of drag
        dragStartPos = e->pos();
    }

    if(e->button()==Qt::RightButton &&
            ((e->modifiers() & Qt::ControlModifier) || (cfg.mouse_is_xterm == 2))) {
        // right click menu
        this->showContextMenu(e);
        e->accept();
        return;
    }
    Mouse_Button button, bcooked;
    button = e->button()==Qt::LeftButton ? MBT_LEFT :
             e->button()==Qt::RightButton ? MBT_RIGHT :
             e->button()==Qt::MidButton ? MBT_MIDDLE : MBT_NOTHING;
    // assert(button!=MBT_NOTHING);
    if(button == MBT_NOTHING)
        return;
    int x = e->x()/fontWidth, y = e->y()/fontHeight, mod=e->modifiers();
    bcooked = translate_button(&cfg, button);

    // detect single/double/triple click
    if (button == MBT_LEFT &&
            !mouseClickTimer.hasExpired(CFG_MOUSE_TRIPLE_CLICK_INTERVAL)) {
        mouseButtonAction =
                mouseButtonAction==MA_CLICK ? MA_2CLK :
                    mouseButtonAction==MA_2CLK ? MA_3CLK :
                        mouseButtonAction==MA_3CLK ? MA_CLICK : MA_NOTHING;
        qDebug()<<__FUNCTION__<<"not expired"<<mouseButtonAction;
    } else
        mouseButtonAction = MA_CLICK;

    term_mouse(term, button, bcooked, mouseButtonAction,
               x,y, mod&Qt::ShiftModifier, mod&Qt::ControlModifier, mod&Qt::AltModifier);
    e->accept();
}

void 	GuiTerminalWindow::mouseReleaseEvent ( QMouseEvent * e )
{
    noise_ultralight(e->x()<<16 | e->y());
    if (!term) return;

    Mouse_Button button, bcooked;
    button = e->button()==Qt::LeftButton ? MBT_LEFT :
             e->button()==Qt::RightButton ? MBT_RIGHT :
             e->button()==Qt::MidButton ? MBT_MIDDLE : MBT_NOTHING;
    //assert(button!=MBT_NOTHING);
    if(button == MBT_NOTHING)
        return;
    int x = e->x()/fontWidth, y = e->y()/fontHeight, mod=e->modifiers();
    bcooked = translate_button(&cfg, button);
    term_mouse(term, button, bcooked, MA_RELEASE,
               x,y, mod&Qt::ShiftModifier, mod&Qt::ControlModifier, mod&Qt::AltModifier);
    e->accept();
}

void GuiTerminalWindow::getClip(wchar_t **p, int *len)
{
    if (p && len) {
        if (clipboard_contents) delete clipboard_contents;
        QString s = QApplication::clipboard()->text();
        clipboard_length = s.length()+1;
        clipboard_contents = new wchar_t[clipboard_length];
        clipboard_length = s.toWCharArray(clipboard_contents);
        clipboard_contents[clipboard_length] = 0;
        *p = clipboard_contents;
        *len = clipboard_length;
    } else {
        // synchronous paste operation
        while (term_paste_pending(term)) {
            int pending = term->paste_pos;
            do {
                term_paste(term);
            } while (term_paste_pending(term) && term->paste_pos - pending < 256);
            if (!term_paste_pending(term))
                break;

            // QT suggests using processEvents is a bad design
            // but hopefully it would fit our purpose
            QCoreApplication::processEvents();
        }
        if (clipboard_contents) delete clipboard_contents;
        clipboard_contents = NULL;
        clipboard_length = 0;
    }
}

void GuiTerminalWindow::requestPaste()
{
    term_do_paste(term);
}

void GuiTerminalWindow::writeClip(wchar_t * data, int * /*attr*/, int len, int /*must_deselect*/)
{
    data[len] = 0;
    QString s = QString::fromWCharArray(data);
    QApplication::clipboard()->setText(s);
}

void 	GuiTerminalWindow::resizeEvent ( QResizeEvent * )
{
    if (viewport()->height() == 0 || viewport()->width() == 0) {
        // skip the spurious resizes during split-pane create/delete/drag-drop
        // we are not missing anything by not resizing, since height/width is 0
        return;
    }

    if (_tmuxMode==TMUX_MODE_CLIENT) {
        wchar_t cmd_resize[128];
        int cmd_resize_len = wsprintf(cmd_resize, L"control set-client-size %d,%d\n",
                                      viewport()->size().width()/fontWidth,
                                      viewport()->size().height()/fontHeight);
        _tmuxGateway->sendCommand(_tmuxGateway, CB_NULL,
                                  cmd_resize, cmd_resize_len);
        // %layout-change tmux command does the actual resize
        return;
    }
    if (term)
        term_size(term, viewport()->size().height()/fontHeight,
                  viewport()->size().width()/fontWidth, cfg.savelines);
}

bool GuiTerminalWindow::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Tab) {
            // ctrl + tab
            keyPressEvent(keyEvent);
            qDebug() << "key" <<keyEvent;
            return true;
        } else if (keyEvent->key() == Qt::Key_Backtab) {
            // ctrl + shift + tab
            keyPressEvent(keyEvent);
            qDebug() << "key" <<keyEvent;
            return true;
        }
    }
    return QAbstractScrollArea::event(event);
}

void GuiTerminalWindow::focusInEvent ( QFocusEvent * )
{
    this->mru_count = ++mainWindow->mru_count_last;
    if (!term) return;
    term_set_focus(term, TRUE);
    term_update(term);
    if (parentSplit)
        mainWindow->tabArea->setTabText(mainWindow->tabArea->currentIndex(),
                                        temp_title);
}

void GuiTerminalWindow::focusOutEvent ( QFocusEvent * )
{
    if (!term) return;
    term_set_focus(term, FALSE);
    term_update(term);
}

void GuiTerminalWindow::setScrollBar(int total, int start, int page)
{
    verticalScrollBar()->setPageStep(page);
    verticalScrollBar()->setRange(0, total-page);
    if (verticalScrollBar()->value()!=start)
        verticalScrollBar()->setValue(start);
}

void GuiTerminalWindow::vertScrollBarAction(int action)
{
    if (!term) return;
    switch(action) {
    case QAbstractSlider::SliderSingleStepAdd:
        term_scroll(term, 0, +1);
        break;
    case QAbstractSlider::SliderSingleStepSub:
        term_scroll(term, 0, -1);
        break;
    case QAbstractSlider::SliderPageStepAdd:
        term_scroll(term, 0, +term->rows/2);
        break;
    case QAbstractSlider::SliderPageStepSub:
        term_scroll(term, 0, -term->rows/2);
        break;
    }
}

void GuiTerminalWindow::vertScrollBarMoved(int value)
{
    if (!term) return;
    term_scroll(term, 1, value);
}

int GuiTerminalWindow::initTmuxControllerMode(char * /*tmux_version*/)
{
    // TODO version check
    assert(_tmuxMode == TMUX_MODE_NONE);

    qDebug()<<"TMUX mode entered";
    _tmuxMode = TMUX_MODE_GATEWAY;
    _tmuxGateway = new TmuxGateway(this);

    return 0;
}

void GuiTerminalWindow::startDetachTmuxControllerMode()
{
    _tmuxMode = TMUX_MODE_GATEWAY_DETACH_INIT;
}

void GuiTerminalWindow::detachTmuxControllerMode()
{
    assert(_tmuxMode == TMUX_MODE_GATEWAY_DETACH_INIT);

    _tmuxGateway->detach();
    delete _tmuxGateway;
    _tmuxGateway = NULL;
    _tmuxMode = TMUX_MODE_NONE;
}

void GuiTerminalWindow::sockError (QAbstractSocket::SocketError socketError)
{
    char errStr[256];
    qstring_to_char(errStr, as->qtsock->errorString(), sizeof(errStr));
    (*as->plug)->closing(as->plug, errStr, socketError, 0);
}

void GuiTerminalWindow::sockDisconnected()
{
    char errStr[256];
    qstring_to_char(errStr, as->qtsock->errorString(), sizeof(errStr));
    (*as->plug)->closing(as->plug, errStr, as->qtsock->error(), 0);
}

void GuiTerminalWindow::closeTerminal()
{
    // be sure to hide the top-right menu
    this->getMainWindow()->toolBarTerminalTop.hideMe();
    if (parentSplit) {
        parentSplit->removeSplitLayout(this);
    }
    mainWindow->closeTerminal(this);
    this->close();
    this->deleteLater();
}

void GuiTerminalWindow::reqCloseTerminal(bool userConfirm)
{
    userClosingTab = true;
    if (!userConfirm && cfg.warn_on_close &&
        !isSockDisconnected &&
        QMessageBox::No == QMessageBox::question(this, "Exit Confirmation?",
                              "Are you sure you want to close this session?",
                              QMessageBox::Yes|QMessageBox::No))
        return;
    this->closeTerminal();
}

void GuiTerminalWindow::on_sessionTitleChange(bool force)
{
    int tabind = mainWindow->getTerminalTabInd(this);
    QString title = "";
    title += QString::number(tabind + 1) + ". ";
    if (!custom_title.isEmpty() && !runtime_title.isEmpty())
        title += custom_title + " - " +runtime_title;
    else if (!custom_title.isEmpty())
        title += custom_title;
    else if (!runtime_title.isEmpty())
        title += runtime_title;
    if (title == temp_title && !force)
        return;
    temp_title = title;
    if (!parentSplit ||
        mainWindow->tabArea->widget(tabind)->focusWidget()==this)
        mainWindow->tabArea->setTabText(tabind, temp_title);
}
