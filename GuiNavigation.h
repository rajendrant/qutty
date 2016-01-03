/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUINAVIGATION_H
#define GUINAVIGATION_H

#include <map>
#include <stdint.h>
#include <QListWidget>

using namespace std;

class GuiMainWindow;
class GuiTerminalWindow;

class GuiTabNavigation : public QListWidget {
  GuiMainWindow *mainWindow;

 public:
  GuiTabNavigation(GuiMainWindow *p);

  void navigateToTab(bool next);
  void acceptNavigation();

 protected:
  void focusOutEvent(QFocusEvent *e);
  void keyPressEvent(QKeyEvent *e);
  void keyReleaseEvent(QKeyEvent *e);
};

class GuiPaneNavigation : public QWidget {
  GuiMainWindow *mainWindow;
  map<uint32_t, GuiTerminalWindow *> mrupanemap;
  uint32_t curr_sel;

 public:
  GuiPaneNavigation(GuiMainWindow *p, bool is_direction_mode = false);

  void navigateToDirectionPane(Qt::Key key);
  void navigateToMRUPane(bool next);
  void acceptNavigation();

 protected:
  void focusOutEvent(QFocusEvent *e);
  void keyReleaseEvent(QKeyEvent *e);
};

#endif  // GUINAVIGATION_H
