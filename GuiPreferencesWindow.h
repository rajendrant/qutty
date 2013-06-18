#ifndef GUIPREFERENCESWINDOW_H
#define GUIPREFERENCESWINDOW_H

#include <QDialog>
#include <QTreeWidget>
#include "GuiMenu.h"

class GuiMainWindow;

namespace Ui {
class GuiPreferencesWindow;
}

class GuiPreferencesWindow : public QDialog
{
    Q_OBJECT

    GuiMainWindow *mainWindow;

    /****** Members for Keyboard Shortcuts Tab *********************/
    int shkey_entered[4];
    int shkey_len;
    bool shkey_changed;
    QTreeWidgetItem *shkey_root_custom_saved_session;
    /***************************************************************/

    void addItemToTree(QTreeWidgetItem *par, qutty_menu_id_t menu_index,
                       const char *text, const char *desc);
    void keysh_saveShortcutChange(QTreeWidgetItem *item);

public:
    explicit GuiPreferencesWindow(GuiMainWindow *parent);
    ~GuiPreferencesWindow();

    bool eventFilter(QObject *src, QEvent *e);

private slots:
    void slot_GuiPreferencesWindow_rejected();

    void on_tree_keysh_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_keysh_custom_saved_session_shortcut_create();
    void on_btn_keysh_clear_clicked();

    void on_buttonBox_accepted();


private:
    Ui::GuiPreferencesWindow *ui;
};

#endif // GUIPREFERENCESWINDOW_H
