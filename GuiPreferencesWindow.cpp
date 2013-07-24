/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QDebug>
#include <QPushButton>
#include <QInputDialog>
#include <QMessageBox>
#include "GuiPreferencesWindow.h"
#include "GuiMainWindow.h"
#include "ui_GuiPreferencesWindow.h"

void GuiPreferencesWindow::addItemToTree(QTreeWidgetItem *par, qutty_menu_id_t menu_index,
                   const char *text, const char *desc)
{
    if (!text)
        text = qutty_menu_actions[menu_index].name;
    if (!desc)
        desc = qutty_menu_actions[menu_index].tooltip;
    QTreeWidgetItem *item = new QTreeWidgetItem(par, menu_index+QTreeWidgetItem::UserType);
    item->setText(0, text);
    item->setToolTip(0, desc);
    item->setText(1, mainWindow->menuGetShortcutById(menu_index).toString());
    par->addChild(item);
}

GuiPreferencesWindow::GuiPreferencesWindow(GuiMainWindow *parent) :
    QDialog(parent),
    mainWindow(parent),
    shkey_len(0),
    shkey_changed(false),
    ui(new Ui::GuiPreferencesWindow)
{
    QTreeWidget *tree;
    QTreeWidgetItem *item;

    memset(shkey_entered, 0, sizeof(shkey_entered));

    ui->setupUi(this);

    /******************** Keyboard Shortcuts Tab ****************************************/
    tree = this->ui->tree_keysh;
    QFont font = tree->font();
    font.setBold(true);
    item = new QTreeWidgetItem(tree, QStringList("New Session"));
    item->setFont(0, font);
    addItemToTree(item, MENU_NEW_TAB);
    addItemToTree(item, MENU_SPLIT_HORIZONTAL, "New Horizontal Split");
    addItemToTree(item, MENU_SPLIT_VERTICAL, "New Vertical Split");
    addItemToTree(item, MENU_NEW_WINDOW);
    item = new QTreeWidgetItem(tree, QStringList("Duplicate Session"));
    item->setFont(0, font);
    addItemToTree(item, MENU_DUPLICATE_SESSION, "Duplicate to new Tab");
    addItemToTree(item, MENU_DUPLICATE_HSPLIT);
    addItemToTree(item, MENU_DUPLICATE_VSPLIT);
    shkey_root_custom_saved_session = new QTreeWidgetItem(tree, QStringList("Saved Session"));
    item = shkey_root_custom_saved_session;
    item->setFont(0, font);
    item = new QTreeWidgetItem(item);
    item->setText(0, "Create custom shortcut to open a saved session");
    QPushButton *button = new QPushButton("Create");
    this->ui->tree_keysh->setItemWidget(item, 1, button);
    connect(button, SIGNAL(clicked()), SLOT(slot_keysh_custom_saved_session_shortcut_create()));
    item = new QTreeWidgetItem(tree, QStringList("Close Session"));
    item->setFont(0, font);
    addItemToTree(item, MENU_CLOSE_SESSION, "Close Session");
    addItemToTree(item, MENU_EXIT, "Close Window");
    item = new QTreeWidgetItem(tree, QStringList("Navigation"));
    item->setFont(0, font);
    addItemToTree(item, MENU_SWITCH_LEFT_TAB);
    addItemToTree(item, MENU_SWITCH_RIGHT_TAB);
    addItemToTree(item, MENU_SWITCH_MRU_TAB);
    addItemToTree(item, MENU_SWITCH_LRU_TAB);
    addItemToTree(item, MENU_SWITCH_UP_PANE);
    addItemToTree(item, MENU_SWITCH_BOTTOM_PANE);
    addItemToTree(item, MENU_SWITCH_LEFT_PANE);
    addItemToTree(item, MENU_SWITCH_RIGHT_PANE);
    addItemToTree(item, MENU_SWITCH_MRU_PANE);
    addItemToTree(item, MENU_SWITCH_LRU_PANE);
    item = new QTreeWidgetItem(tree, QStringList("Miscellaneous"));
    item->setFont(0, font);
    addItemToTree(item, MENU_CHANGE_SETTINGS);
    addItemToTree(item, MENU_RENAME_TAB);
    addItemToTree(item, MENU_PASTE);
    addItemToTree(item, MENU_MENUBAR);
    addItemToTree(item, MENU_FULLSCREEN);
    addItemToTree(item, MENU_ALWAYSONTOP);
    addItemToTree(item, MENU_PREFERENCES);
    addItemToTree(item, MENU_FIND);
    addItemToTree(item, MENU_FIND_NEXT);
    addItemToTree(item, MENU_FIND_PREVIOUS);
    item = new QTreeWidgetItem(tree, QStringList("Import and Export"));
    item->setFont(0, font);
    addItemToTree(item, MENU_IMPORT_FILE);
    addItemToTree(item, MENU_IMPORT_REGISTRY);
    addItemToTree(item, MENU_EXPORT_FILE);

    /***** setup custom saved session list ******/
    auto it_begin = qutty_config.menu_action_list.lower_bound(MENU_CUSTOM_OPEN_SAVED_SESSION);
    auto it_end = qutty_config.menu_action_list.upper_bound(MENU_CUSTOM_OPEN_SAVED_SESSION_END);
    for (auto it = it_begin; it != it_end; ++it) {
        keyshAddCustomSavedSessionToTree(it->second.str_data, it->second.int_data,
                                         it->second.shortcut);
    }
    tree->expandAll();
    tree->resizeColumnToContents(0);
    tree->setMinimumHeight(200);
    this->ui->le_keysh_shortcut->installEventFilter(this);
    ui->le_keysh_shortcut->setDisabled(true);
    /*************************************************************************************/

    this->ui->tabWidget->removeTab(1);

    // resize to minimum needed dimension
    this->resize(0, 0);

    this->connect(this, SIGNAL(accepted()), SLOT(slot_GuiPreferencesWindow_accepted()));
    this->connect(this, SIGNAL(rejected()), SLOT(slot_GuiPreferencesWindow_rejected()));
}

GuiPreferencesWindow::~GuiPreferencesWindow()
{
    delete ui;
}

void GuiPreferencesWindow::slot_GuiPreferencesWindow_rejected()
{
    this->close();
    this->deleteLater();
}

bool GuiPreferencesWindow::eventFilter(QObject *src, QEvent *e)
{
    if (src == this->ui->le_keysh_shortcut) {
        if (e->type() == QEvent::KeyPress) {
            QKeyEvent *k = static_cast<QKeyEvent*>(e);
            if (k->key() == Qt::Key_Control || k->key() == Qt::Key_Shift ||
                k->key() == Qt::Key_Alt || shkey_len >= 4)
                return true;
            if (shkey_len==0)
                memset(shkey_entered, 0, sizeof(shkey_entered));
            shkey_entered[shkey_len++] = k->key() | k->modifiers();
            ui->le_keysh_shortcut->setText(QKeySequence(shkey_entered[0], shkey_entered[1],
                    shkey_entered[2], shkey_entered[3]).toString());
            return true;
        }
        return false;
    }
    return QDialog::eventFilter(src, e);
}

void GuiPreferencesWindow::on_tree_keysh_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    int menu_ind;
    keysh_saveShortcutChange(previous);
    ui->lbl_keysh_desc->clear();
    ui->le_keysh_shortcut->clear();
    ui->le_keysh_shortcut->setDisabled(true);
    shkey_len = 0;
    memset(shkey_entered, 0, sizeof(shkey_entered));
    menu_ind = !current ? -1 : current->type() - QTreeWidgetItem::UserType;
    if (menu_ind >= 0 && menu_ind < MENU_MAX_ACTION) {
        ui->lbl_keysh_desc->setText(current->toolTip(0));
        ui->le_keysh_shortcut->setText(current->text(1));
        ui->le_keysh_shortcut->setEnabled(true);
        QKeySequence keyseq(current->text(1));
        for (uint i=0; i<keyseq.count(); i++)
            shkey_entered[i] = keyseq[i];
        shkey_len = 0;
    }
}

void GuiPreferencesWindow::keysh_saveShortcutChange(QTreeWidgetItem *item)
{
    qutty_menu_id_t menu_ind;
    menu_ind = (qutty_menu_id_t)(!item ? -1 : item->type() - QTreeWidgetItem::UserType);
    if (menu_ind >= 0 && menu_ind < MENU_MAX_ACTION) {
        QString newseq = QKeySequence(shkey_entered[0], shkey_entered[1],
                                      shkey_entered[2], shkey_entered[3]).toString();
        QString oldseq = item->text(1);
        if (oldseq != newseq) {
            item->setText(1, newseq);
            QFont f = item->font(1);
            f.setBold(newseq != mainWindow->menuGetShortcutById(menu_ind).toString());
            item->setFont(1, f);
            shkey_changed = true;
        }
    }
}

void GuiPreferencesWindow::on_btn_keysh_clear_clicked()
{
    ui->le_keysh_shortcut->clear();
    shkey_len = 0;
    memset(shkey_entered, 0, sizeof(shkey_entered));
}

void GuiPreferencesWindow::slot_keysh_custom_saved_session_shortcut_create()
{
    QString session, opentype;
    QStringList sesslist, opentypelist;
    int opentypeind;
    bool ok;

    sesslist.append(tr("--Select Session--"));
    for(auto it=qutty_config.config_list.begin();
        it != qutty_config.config_list.end(); ++it) {
        if (it->first == QUTTY_DEFAULT_CONFIG_SETTINGS)
            continue;
        sesslist.append(it->first);
    }
    session = QInputDialog::getItem(this, "Select session",
                        "Select a session to create shortcut for",
                        sesslist, 0, false, &ok);
    if (!ok || session == sesslist[0])
        return;
    opentypelist.append(tr("--Select open mode--"));
    opentypelist.append(tr("Open in a New Tab"));
    opentypelist.append(tr("Open in Horizontal Split"));
    opentypelist.append(tr("Open in Vertical Split"));
    opentype = QInputDialog::getItem(this, tr("Select open mode"),
                                     tr("How do you want to open the session"),
                                     opentypelist, 0, false, &ok);
    if (!ok || opentype == opentypelist[0])
        return;

    opentypeind = opentype==opentypelist[1] ? GuiBase::TYPE_LEAF :
                  opentype==opentypelist[2] ? GuiBase::TYPE_HORIZONTAL :
                                              GuiBase::TYPE_VERTICAL;
    keyshAddCustomSavedSessionToTree(session, opentypeind, QKeySequence());
}

void GuiPreferencesWindow::keyshAddCustomSavedSessionToTree(QString session, int opentypeind, QKeySequence key)
{
    QString opentypestr = opentypeind==GuiBase::TYPE_LEAF ? "New Tab" :
                          opentypeind==GuiBase::TYPE_HORIZONTAL ? "Horizontal Split" :
                                           "Vertical Split";
    QTreeWidgetItem *item = new QTreeWidgetItem(QTreeWidgetItem::UserType + MENU_CUSTOM_OPEN_SAVED_SESSION);
    item->setText(0, "Open '" + session + "' in " + opentypestr);
    item->setToolTip(0, "Custom keyboard shortcut to open saved session '" + session +
                     "' in " + opentypestr);
    item->setData(0, QTreeWidgetItem::UserType, session);
    item->setData(0, QTreeWidgetItem::UserType+1, opentypeind);
    item->setText(1, key.toString());
    shkey_root_custom_saved_session->insertChild(shkey_root_custom_saved_session->childCount()-1,
                                                item);
}

void GuiPreferencesWindow::slot_GuiPreferencesWindow_accepted()
{
    bool is_config_changed = false;
    QTreeWidgetItem *root, *ch;

    /******************** Keyboard Shortcuts Tab ****************************************/
    if (!shkey_changed)
        goto keyboard_shortcut_done;
    root = ui->tree_keysh->invisibleRootItem();
    for(int i = 0; i < root->childCount(); i++) {
        QTreeWidgetItem *item = root->child(i);
        if (item == shkey_root_custom_saved_session)
            continue;
        for(int j = 0; j < item->childCount(); j++) {
            ch = item->child(j);
            qutty_menu_id_t menu_ind = (qutty_menu_id_t)(ch->type() - QTreeWidgetItem::UserType);
            if (menu_ind < 0 || menu_ind >= MENU_MAX_ACTION)
                continue;
            QKeySequence newseq = ch->text(1);
            if (newseq != mainWindow->menuGetShortcutById(menu_ind)) {
                // we have a new shortcut configured
                mainWindow->menuSetShortcutById(menu_ind, newseq);
                if (newseq != QString::fromLatin1(qutty_menu_actions[menu_ind].key)) {
                    // shortcut needs to be saved in config
                    QtMenuActionConfig actioncfg(menu_ind, newseq);
                    qutty_config.menu_action_list.erase(menu_ind);
                    qutty_config.menu_action_list.insert(std::make_pair(menu_ind, actioncfg));
                } else {
                    // shortcut needs to be removed from config
                    qutty_config.menu_action_list.erase(menu_ind);
                }
                is_config_changed = true;
            }
        }
    }
    // clear the saved-session in config & recreate
    {
    auto it_begin = qutty_config.menu_action_list.lower_bound(MENU_CUSTOM_OPEN_SAVED_SESSION);
    auto it_end = qutty_config.menu_action_list.upper_bound(MENU_CUSTOM_OPEN_SAVED_SESSION_END);
    qutty_config.menu_action_list.erase(it_begin, it_end);
    }
    for(int i = 0; i < shkey_root_custom_saved_session->childCount()-1; i++) {
        QTreeWidgetItem *ch = shkey_root_custom_saved_session->child(i);
        int menu_ind = ch->type() - QTreeWidgetItem::UserType;
        QString session = ch->data(0, QTreeWidgetItem::UserType).toString();
        int opentypemode = ch->data(0, QTreeWidgetItem::UserType+1).toInt();
        if (menu_ind != MENU_CUSTOM_OPEN_SAVED_SESSION || ch->text(1).isEmpty() ||
            session.isEmpty() || opentypemode < 1 || opentypemode > 4)
            continue;
        QKeySequence newseq = ch->text(1);
        QtMenuActionConfig action(MENU_CUSTOM_OPEN_SAVED_SESSION+i, newseq, "",
                                  session, opentypemode);
        qutty_config.menu_action_list.insert(std::make_pair(MENU_CUSTOM_OPEN_SAVED_SESSION+i,
                                                action));
    }
    mainWindow->initializeCustomSavedSessionShortcuts();
    is_config_changed = true;
    /*************************************************************************************/

keyboard_shortcut_done:

    if (is_config_changed)
        qutty_config.saveConfig();

    return;
}

void GuiPreferencesWindow::on_btn_ok_clicked()
{
    QTreeWidgetItem *root, *ch;
    map<QKeySequence, QTreeWidgetItem*> dup_find;

    /******************** Keyboard Shortcuts Tab ****************************************/
    keysh_saveShortcutChange(ui->tree_keysh->currentItem());
    if (!shkey_changed)
        goto keyboard_shortcut_done;
    root = ui->tree_keysh->invisibleRootItem();
    // find duplicates in keyboard shortcuts
    for(int i = 0; i < root->childCount(); i++) {
        QTreeWidgetItem *item = root->child(i);
        for(int j = 0; j < item->childCount(); j++) {
            ch = item->child(j);
            int menu_ind = ch->type() - QTreeWidgetItem::UserType;
            if (menu_ind < 0 || menu_ind >= MENU_MAX_ACTION)
                continue;
            QKeySequence newseq = ch->text(1);
            if (newseq.isEmpty())
                continue;
            auto it = dup_find.find(newseq);
            if(it != dup_find.end()) {
                QString msg("The following commands have duplicate keyboard shortcut configured.\n"
                            " * " + it->second->text(0) + "\n" +
                            " * " + ch->text(0) + "\n");
                QMessageBox::critical(this, tr("Duplicate keyboard shortcut"), msg);
                //this->reject();
                return;
            }
            dup_find[newseq] = ch;
        }
    }

keyboard_shortcut_done:

    this->accept();
}

void GuiPreferencesWindow::on_btn_cancel_clicked()
{
    this->reject();
}

void GuiPreferencesWindow::on_menu_cb_sel_currentIndexChanged(int index)
{
    if (index == 1) {
        ui->menu_lbl_desc->setText(tr("This menu is shown when right-clicking on a Tab in the Tabbar\n"
                                      "or when Ctrl + right-clicking inside a Terminal."));
    } else if (index == 2) {
        ui->menu_lbl_desc->setText(tr("This menu is shown when the drop-down button at top-right corner is clicked."));
    } else {
        ui->menu_lbl_desc->clear();
    }
}
