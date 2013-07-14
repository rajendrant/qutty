#ifndef GUICOMPACTSETTINGSWINDOW_H
#define GUICOMPACTSETTINGSWINDOW_H

#include <QDialog>
#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QListWidget>
#include <QComboBox>
#include <QIODevice>
#include <string>
#include "GuiBase.h"
#include "QtConfig.h"
#include "GuiSettingsWindow.h"
#include <QMessageBox>
#include "QtComboBoxWithTreeView.h"

class GuiMainWindow;

class GuiCompactSettingsWindow : public QDialog
{
    Q_OBJECT

    QtComboBoxWithTreeView *cb_session_list;
    QComboBox *cb_connection_type;
    QLineEdit *le_hostname;
    GuiBase::SplitType openMode;

public:
    explicit GuiCompactSettingsWindow(QWidget *parent, GuiBase::SplitType openmode = GuiBase::TYPE_LEAF);

signals:
    void signal_on_open(Config cfg, GuiBase::SplitType splittype);
    void signal_on_close();
    void signal_on_detail(Config cfg, GuiBase::SplitType splittype);

public slots:
    void on_open_clicked();
    void on_close_clicked();
    void on_details_clicked();
    void on_cb_session_list_activated(int);
};

#endif // GUICOMPACTSETTINGSWINDOW_H
