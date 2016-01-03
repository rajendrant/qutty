#ifndef GUITEXTFILTERWINDOW_H
#define GUITEXTFILTERWINDOW_H

#include <QToolBar>
#include <QLineEdit>
#include <QPlainTextEdit>

#include "GuiMainWindow.h"
#include "QtCompleterWithAdvancedCompletion.h"

class GuiTextFilterWindow : public QToolBar {
  Q_OBJECT

  GuiMainWindow *mainWnd;
  QLineEdit *filter;
  QtCompleterWithAdvancedCompletion *completer;
  QToolButton *edit, *save;
  QPlainTextEdit *editor;
  QString label;
  bool is_editable;

  QStringList getCompletions();
  void setCompletions(QString str);

 public:
  explicit GuiTextFilterWindow(GuiMainWindow *p, bool isEditable, QString lbl);
  virtual ~GuiTextFilterWindow() {}

  void init();

  QSize sizeHint() const;

 public slots:
  void on_text_completion_activated(QString str);
  void on_deactivated();
  void on_editList();
  void on_saveList();
};

#endif  // GUITEXTFILTERWINDOW_H
