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
extern "C" {
#include "putty.h"
}
#include "GuiTerminalWindow.h"

GuiTerminalWindow::GuiTerminalWindow(QWidget *parent) :
    QWidget(parent)
{
    setWindowState(Qt::WindowMaximized);
    setWindowTitle(tr("QuTTY"));

    QPalette pal(palette());
    // set black background
    pal.setColor(QPalette::Background, Qt::black);
    pal.setColor(QPalette::Foreground, Qt::white);
    QWidget::setAutoFillBackground(true);
    QWidget::setPalette(pal);

    QWidget::setFocusPolicy(Qt::StrongFocus);
    _any_update = false;

    termrgn = QRegion();
}

void GuiTerminalWindow::keyPressEvent ( QKeyEvent *e )
{
    qDebug()<<"you pressed "<<e->text()<<" "<<e->count()<<" "<<e->key()<<" "<<e->modifiers();

    char buf[16];
    int len = TranslateKey(&term->cfg, term, e, buf);
    assert(len<16);
    if (len>0 || len==-2) {
        term_nopaste(term);
        term_seen_key_event(term);
        ldisc_send(ldisc, buf, len, 1);
        //show_mouseptr(0);
        qDebug()<<"keypress"<<len<<buf[0]<<buf[1];
    } else if(len==-1) {
        wchar_t bufwchar[16];
        len = e->text().toWCharArray(bufwchar);
        assert(len<16);
        term_nopaste(term);
        term_seen_key_event(term);
        luni_send(ldisc, bufwchar, len, 1);
        qDebug()<<"keypress wchar"<<bufwchar[0]<<len<<term->cr_lf_return;
        //ldisc_send(ldisc, buf, 1, 1);
    }
}

void GuiTerminalWindow::keyReleaseEvent ( QKeyEvent * e )
{
    qDebug()<<"you released "<<e->text()<<" "<<e->count()<<" "<<e->key()<<" "<<e->modifiers();
}

void GuiTerminalWindow::closeEvent(QCloseEvent *closeEvent)
{
    /* HACK: most likely Ctrl+W is pressed. ignore it */
    if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
        qDebug()<< "closing request "<<closeEvent->type()<<" "<<closeEvent->spontaneous();
        closeEvent->ignore();
    }
    closeEvent->accept();
}

void GuiTerminalWindow::readyRead ()
{
    char buf[20480];
    int len = qtsock->read(buf, sizeof(buf));
    char ddd[20480*10];
    for(int i=0,j=0;i<len;i++)
        j+=sprintf(ddd+j, "%u ", (unsigned char)buf[i], buf[i]);
    qDebug()<<"readyRead"<<len<<ddd;
    (*as->plug)->receive(as->plug, 0, buf, len);

    if(qtsock->bytesAvailable()>0) readyRead();
}

void GuiTerminalWindow::paintEvent (QPaintEvent *e)
{
    qDebug()<<__FUNCTION__;
    QPainter painter(this);

    if(!term)
        return;

    for(int i=0; i<e->region().rects().size(); i++) {
        const QRect &r = e->region().rects().at(i);
        int row = r.top()/fontHeight;
        int colstart = r.left()/fontWidth;
        int rowend = (r.bottom()+1)/fontHeight;
        int colend = (r.right()+1)/fontWidth;
        for(; row<rowend; row++) {
            for(int col=colstart; col<colend; ) {
                int attr = term->dispstr_attr[row][col];
                int coldiff = col+1;
                for(;attr==term->dispstr_attr[row][coldiff]; coldiff++);
                QString str = QString::fromWCharArray(&term->dispstr[row][col], coldiff-col);
                paintText(painter, row, col, str, attr);
                col = coldiff;
            }
        }
    }
}

void GuiTerminalWindow::paintText(QPainter &painter, int row, int col, QString str, unsigned long attr)
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
                     row*fontHeight+_fontMetrics->ascent(),
                     str);
}

int GuiTerminalWindow::from_backend(int is_stderr, const char *data, int len)
{
    qDebug()<<"from_backend"<<data;
    return term_data(term, is_stderr, data, len);
}

void GuiTerminalWindow::preDrawTerm()
{
    qDebug()<<__FUNCTION__;
    termrgn = QRegion();
}

void GuiTerminalWindow::drawTerm()
{
    qDebug()<<__FUNCTION__;
    this->repaint(termrgn);
    qDebug()<<__FUNCTION__<<"end";
}

void GuiTerminalWindow::drawText(int row, int col, wchar_t *ch, int len, unsigned long attr, int lattr)
{
    if (attr & TATTR_COMBINING) {
        return;
    }
    ch[len] = '\0';
    QString str = QString::fromWCharArray(ch);
    termrgn |= QRect(col*fontWidth, row*fontHeight, fontWidth*len, fontHeight);
    qDebug()<<"drawText"<<row<<col<<attr<<lattr<<QString::fromWCharArray(ch);
}

void GuiTerminalWindow::setTermFont(FontSpec *f)
{
    if (_font) delete _font;
    if (_fontMetrics) delete _fontMetrics;

    _font = new QFont(f->name, f->height);
    _font->setStyleHint(QFont::TypeWriter);
    QWidget::setFont(*_font);
    _fontMetrics = new QFontMetrics(*_font);

    fontWidth = _fontMetrics->width(QChar('a'));
    fontHeight = _fontMetrics->height();

    qDebug()<<_font->family()<<" "<<_font->styleHint()<<" "<<_font->style();
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
    e->accept();
}
//#define (e) e->button()&Qt::LeftButton
void 	GuiTerminalWindow::mouseMoveEvent ( QMouseEvent * e )
{
    Mouse_Button button, bcooked;
    button = e->buttons()&Qt::LeftButton ? MBT_LEFT :
             e->buttons()&Qt::RightButton ? MBT_RIGHT :
             e->buttons()&Qt::MidButton ? MBT_MIDDLE : MBT_NOTHING;
    assert(button!=MBT_NOTHING);
    int x = e->x()/fontWidth, y = e->y()/fontHeight, mod=e->modifiers();
    bcooked = translate_button(&cfg, button);
    qDebug()<<__FUNCTION__<<x<<y<<mod<<button<<bcooked;
    term_mouse(term, button, bcooked, MA_DRAG,
               x,y, mod&Qt::ShiftModifier, mod&Qt::ControlModifier, mod&Qt::AltModifier);/*
               e->x()/fontWidth, e->y()/fontHeight,
               e->modifiers()&Qt::ShiftModifier,
               e->modifiers()&Qt::ControlModifier,
               e->modifiers()&Qt::AltModifier);*/
    qDebug()<<__FUNCTION__<<"end";
    e->accept();
}
void 	GuiTerminalWindow::mousePressEvent ( QMouseEvent * e )
{
    if(e->button()==Qt::RightButton &&
            ((e->modifiers() & Qt::ControlModifier) || (cfg.mouse_is_xterm == 2))) {
        // TODO right click menu
    }
    Mouse_Button button, bcooked;
    button = e->button()==Qt::LeftButton ? MBT_LEFT :
             e->button()==Qt::RightButton ? MBT_RIGHT :
             e->button()==Qt::MidButton ? MBT_MIDDLE : MBT_NOTHING;
    assert(button!=MBT_NOTHING);
    int x = e->x()/fontWidth, y = e->y()/fontHeight, mod=e->modifiers();
    bcooked = translate_button(&cfg, button);
    qDebug()<<__FUNCTION__<<x<<y<<mod<<button<<bcooked;
    term_mouse(term, button, bcooked, MA_CLICK,
               x,y, mod&Qt::ShiftModifier, mod&Qt::ControlModifier, mod&Qt::AltModifier);/*
               e->x()/fontWidth, e->y()/fontHeight,
               e->modifiers()&Qt::ShiftModifier,
               e->modifiers()&Qt::ControlModifier,
               e->modifiers()&Qt::AltModifier);*/
    qDebug()<<__FUNCTION__<<"end";
    e->accept();
}
void 	GuiTerminalWindow::mouseReleaseEvent ( QMouseEvent * e )
{
    Mouse_Button button, bcooked;
    button = e->button()==Qt::LeftButton ? MBT_LEFT :
             e->button()==Qt::RightButton ? MBT_RIGHT :
             e->button()==Qt::MidButton ? MBT_MIDDLE : MBT_NOTHING;
    assert(button!=MBT_NOTHING);
    int x = e->x()/fontWidth, y = e->y()/fontHeight, mod=e->modifiers();
    bcooked = translate_button(&cfg, button);
    qDebug()<<__FUNCTION__<<x<<y<<mod<<button<<bcooked;
    term_mouse(term, button, bcooked, MA_RELEASE,
               x,y, mod&Qt::ShiftModifier, mod&Qt::ControlModifier, mod&Qt::AltModifier);
    qDebug()<<__FUNCTION__<<"end";
    e->accept();
}

void GuiTerminalWindow::getClip(wchar_t **p, int *len)
{
    static wchar_t *clipboard_contents = NULL;
    static int clipboard_length = 0;
    if (p && len) {
        if (clipboard_contents) delete clipboard_contents;
        QString s = QApplication::clipboard()->text();
        qDebug()<<"clipboard"<<s;
        clipboard_length = s.length()+1;
        clipboard_contents = new wchar_t[clipboard_length];
        clipboard_length = s.toWCharArray(clipboard_contents);
        clipboard_contents[clipboard_length] = 0;
        *p = clipboard_contents;
        *len = clipboard_length;
    } else {
        qDebug()<<"clipboard clear";
        if (clipboard_contents) delete clipboard_contents;
        clipboard_contents = NULL;
        clipboard_length = 0;
    }
}

void GuiTerminalWindow::requestPaste()
{
    term_do_paste(term);
}

void GuiTerminalWindow::writeClip(wchar_t * data, int *attr, int len, int must_deselect)
{
    data[len] = 0;
    QString s = QString::fromWCharArray(data);
    QApplication::clipboard()->setText(s);
}

void 	GuiTerminalWindow::resizeEvent ( QResizeEvent * e )
{
    if (term)
        term_size(term, e->size().height()/fontHeight, e->size().width()/fontWidth, cfg.savelines);
}

bool GuiTerminalWindow::event(QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Tab) {
            keyPressEvent(keyEvent);
            return true;
        }
    }
    return QWidget::event(event);
}

void GuiTerminalWindow::focusInEvent ( QFocusEvent * e )
{
    qDebug()<<__FUNCTION__;
    term_set_focus(term, TRUE);
    term_update(term);
}

void GuiTerminalWindow::focusOutEvent ( QFocusEvent * e )
{
    qDebug()<<__FUNCTION__;
    term_set_focus(term, FALSE);
    term_update(term);
}
