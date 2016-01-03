#include "guitextfilterwindow.h"

#include <QToolButton>

#include "serialize/QtWebPluginMap.h"
#include "GuiTerminalWindow.h"

GuiTextFilterWindow::GuiTextFilterWindow(GuiMainWindow *p, bool isEditable, QString lbl)
    : QToolBar(p), mainWnd(p), label(lbl), is_editable(isEditable) {
  filter = new QLineEdit();
  addWidget(filter);

  setAutoFillBackground(true);
  adjustSize();

  completer = new QtCompleterWithAdvancedCompletion(filter);
  completer->set_always_show_popup(true);
  connect(completer, SIGNAL(activated(QString)), this, SLOT(on_text_completion_activated(QString)));
  connect(completer, SIGNAL(deactivated()), this, SLOT(on_deactivated()));

  if (is_editable) {
    edit = new QToolButton();
    edit->setText("Edit");
    connect(edit, SIGNAL(clicked()), this, SLOT(on_editList()));
    addWidget(edit);
  } else {
    edit = NULL;
  }

  filter->setFocus();

  editor = NULL;
  save = NULL;
}

void GuiTextFilterWindow::init() {
  auto v = getCompletions();
  completer->setModel(v);
  completer->completionSearchString("");
  adjustSize();
}

QSize GuiTextFilterWindow::sizeHint() const { return QSize(mainWnd->width() / 3, height()); }

QStringList GuiTextFilterWindow::getCompletions() {
  QStringList completions;
  for (auto &e : qutty_web_plugin_map.hash_map[label]) completions.append(e.toString());
  return completions;
}

void GuiTextFilterWindow::setCompletions(QString str) {
  QVariantList varl;
  for (auto &e : str.split("\n")) varl.append(e);
  qutty_web_plugin_map.hash_map[label] = varl;
  qutty_web_plugin_map.save();
}

void GuiTextFilterWindow::on_text_completion_activated(QString str) {
  GuiTerminalWindow *t = mainWnd->getCurrentTerminal();

  wchar_t *buf = new wchar_t[str.length() + 1];
  str.toWCharArray(buf);
  luni_send(t->term->ldisc, buf, str.length(), 0);
  delete buf;

  on_deactivated();
}

void GuiTextFilterWindow::on_deactivated() {
  GuiTerminalWindow *t;

  mainWnd->textFilterWnd = NULL;
  this->deleteLater();

  if ((t = mainWnd->getCurrentTerminal())) t->setFocus();
}

void GuiTextFilterWindow::on_editList() {
  filter->deleteLater();
  edit->deleteLater();

  editor = new QPlainTextEdit();
  addWidget(editor);

  save = new QToolButton();
  save->setText("Save");
  connect(save, SIGNAL(clicked()), this, SLOT(on_saveList()));
  addWidget(save);

  QString v;
  for (auto &e : getCompletions()) v.append(e + "\n");
  editor->setPlainText(v);

  editor->setFocus();
  setMinimumHeight(mainWnd->height() / 3);
  adjustSize();
}

void GuiTextFilterWindow::on_saveList() {
  setCompletions(editor->toPlainText());
  this->on_deactivated();
}
