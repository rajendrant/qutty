/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include "QtCommon.h"
#include "GuiTerminalWindow.h"
#include <QTextCodec>

/* Dummy routine, only required in plink. */
void ldisc_update(void * /*frontend*/, int /*echo*/, int /*edit*/) {}

void frontend_keypress(void * /*handle*/) {
  /*
   * Keypress termination in non-Close-On-Exit mode is not
   * currently supported in PuTTY proper, because the window
   * always has a perfectly good Close button anyway. So we do
   * nothing here.
   */
  return;
}

void set_title(void *frontend, const char *title) {
  assert(frontend);
  GuiTerminalWindow *f = static_cast<GuiTerminalWindow *>(frontend);
  f->setSessionTitle(QString::fromLatin1(title));
}

void set_icon(void * /*frontend*/, char * /*str*/) {}
void set_sbar(void *frontend, int total, int start, int page) {
  GuiTerminalWindow *f = static_cast<GuiTerminalWindow *>(frontend);
  f->setScrollBar(total, start, page);
}

Context get_ctx(void *frontend) {
  GuiTerminalWindow *f = static_cast<GuiTerminalWindow *>(frontend);
  f->preDrawTerm();
  return frontend;
}
void free_ctx(Context ctx) {
  GuiTerminalWindow *f = static_cast<GuiTerminalWindow *>(ctx);
  f->drawTerm();
}

printer_job *printer_start_job(char * /*printer*/) { return NULL; }
void printer_job_data(printer_job * /*pj*/, void * /*data*/, int /*len*/) {}
void printer_finish_job(printer_job * /*pj*/) {}

/*
 * Report the window's position, for terminal reports.
 */
void get_window_pos(void * /*frontend*/, int *x, int *y) {
  *x = 1;
  *y = 1;
}

/*
 * Report the window's pixel size, for terminal reports.
 */
void get_window_pixels(void * /*frontend*/, int *x, int *y) {
  *x = 80 * 10;
  *y = 24 * 10;
}

/*
 * Return the window or icon title.
 */
char *get_window_title(void * /*frontend*/, int icon) {
  return (char *)(icon ? "icon_name" : "window_name");
}

// void logtraffic(void *handle, unsigned char c, int logmode){}
// void logflush(void *handle) {}

int is_iconic(void * /*frontend*/) { return 0; }
void set_iconic(void * /*frontend*/, int /*iconic*/) {}

void request_resize(void * /*frontend*/, int /*w*/, int /*h*/) {}

void set_raw_mouse_mode(void * /*frontend*/, int /*activate*/) {}

void do_beep(void * /*frontend*/, int /*mode*/) {}

void set_zorder(void * /*frontend*/, int /*top*/) {}

void move_window(void * /*frontend*/, int /*x*/, int /*y*/) {}

void write_clip(void *frontend, wchar_t *data, int *attr, int len, int must_deselect) {
  GuiTerminalWindow *f = static_cast<GuiTerminalWindow *>(frontend);
  f->writeClip(data, attr, len, must_deselect);
}

void get_clip(void *frontend, wchar_t **p, int *len) {
  GuiTerminalWindow *f = static_cast<GuiTerminalWindow *>(frontend);
  f->getClip(p, len);
}

void request_paste(void *frontend) {
  GuiTerminalWindow *f = static_cast<GuiTerminalWindow *>(frontend);
  f->requestPaste();
}

void sys_cursor(void * /*frontend*/, int /*x*/, int /*y*/) {}

void refresh_window(void *frontend) {
  GuiTerminalWindow *f = static_cast<GuiTerminalWindow *>(frontend);
  f->repaint();
}

void set_zoomed(void * /*frontend*/, int /*zoomed*/) {}

// int mk_wcwidth(wchar_t ucs){qDebug()<<"NOT_IMPL"<<__FUNCTION__;return 0;}
// int mk_wcwidth_cjk(wchar_t ucs){qDebug()<<"NOT_IMPL"<<__FUNCTION__;return 0;}

void palette_set(void * /*frontend*/, int, int, int, int) {
  qDebug() << "NOT_IMPL" << __FUNCTION__;
}
void palette_reset(void * /*frontend*/) { qDebug() << "NOT_IMPL" << __FUNCTION__; }

int is_dbcs_leadbyte(int /*codepage*/, char /*byte*/) {
  qDebug() << "NOT_IMPL" << __FUNCTION__;
  return 0;
}
int mb_to_wc(int /*codepage*/, int /*flags*/, char *mbstr, int mblen, wchar_t *wcstr, int /*wclen*/,
             struct unicode_data *ucsdata) {
  if (!ucsdata->encoder) return 0;
  QTextCodec *codec = (QTextCodec *)ucsdata->encoder;
  return codec->toUnicode(mbstr, mblen).toWCharArray(wcstr);
}

int wc_to_mb(int /*codepage*/, int /*flags*/, wchar_t *wcstr, int wclen, char *mbstr, int mblen,
             char * /*defchr*/, int * /*defused*/, struct unicode_data *ucsdata) {
  if (!ucsdata->encoder) return 0;
  QTextCodec *codec = (QTextCodec *)ucsdata->encoder;
  QByteArray mbarr = codec->fromUnicode(QString::fromWCharArray(wcstr, wclen));
  qstrncpy(mbstr, mbarr.constData(), mblen);
  return mbarr.length();
}

int char_width(Context /*ctx*/, int /*uc*/) {
  // qDebug()<<"NOT_IMPL"<<uc<<__FUNCTION__;
  return 1;
}
void do_text(Context ctx, int col, int row, wchar_t *ch, int len, unsigned long attr, int lattr) {
  GuiTerminalWindow *frontend = static_cast<GuiTerminalWindow *>(ctx);
  frontend->drawText(row, col, ch, len, attr, lattr);
}

void do_cursor(Context /*ctx*/, int /*x*/, int /*y*/, wchar_t * /*text*/, int /*len*/,
               unsigned long /*attr*/, int /*lattr*/) {}

/*void logevent(const char *s) {
    return logevent(NULL, s);
//    return logevent(ssh->frontend, s);
}*/
