/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QDebug>
#include <QPushButton>
#include <QInputDialog>
#include "GuiPreferencesWindow.h"
#include "GuiMainWindow.h"
#include "ui_GuiPreferencesWindow.h"

void GuiPreferencesWindow::addItemToTree(QTreeWidgetItem *par, qutty_menu_id_t menu_index,
                   const char *text, const char *desc)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(par, menu_index+QTreeWidgetItem::UserType);
    item->setText(0, text);
    item->setToolTip(0, desc);
    item->setText(1, mainWindow->menuCommonActions[menu_index]->shortcut().toString());
    par->addChild(item);
}

GuiPreferencesWindow::GuiPreferencesWindow(GuiMainWindow *parent) :
    QDialog(parent),
    ui(new Ui::GuiPreferencesWindow),
    mainWindow(parent),
    shkey_entered({0,0,0,0}),
    shkey_len(0),
    shkey_changed(false)
{
    QTreeWidget *tree;
    QTreeWidgetItem *item;

    ui->setupUi(this);

    /******************** Keyboard Shortcuts Tab ****************************************/
    tree = this->ui->tree_keysh;
    QFont font = tree->font();
    font.setBold(true);
    item = new QTreeWidgetItem(tree, QStringList("New Session"));
    item->setFont(0, font);
    addItemToTree(item, MENU_NEW_TAB, "New Tab", "");
    addItemToTree(item, MENU_SPLIT_HORIZONTAL, "New Horizontal Split", "");
    addItemToTree(item, MENU_SPLIT_VERTICAL, "New Vertical Split", "");
    addItemToTree(item, MENU_NEW_WINDOW, "New Window", "");
    item = new QTreeWidgetItem(tree, QStringList("Duplicate Session"));
    item->setFont(0, font);
    addItemToTree(item, MENU_DUPLICATE_SESSION, "Duplicate to new Tab", "12345");
    addItemToTree(item, MENU_DUPLICATE_HSPLIT, "Duplicate to new HSplit", "");
    addItemToTree(item, MENU_DUPLICATE_VSPLIT, "Duplicate to new VSplit", "");
    shkey_root_custom_saved_session = new QTreeWidgetItem(tree, QStringList("Saved Session"));
    item = shkey_root_custom_saved_session;
    item->setFont(0, font);
    item = new QTreeWidgetItem(item);
    item->setText(0, "Create custom shortcut to open a saved session");
    QPushButton *button = new QPushButton("Create");
    this->ui->tree_keysh->setItemWidget(item, 1, button);
    connect(button, SIGNAL(clicked()), SLOT(on_keysh_custom_saved_session_shortcut_create()));
    item = new QTreeWidgetItem(tree, QStringList("Close Session"));
    item->setFont(0, font);
    addItemToTree(item, MENU_CLOSE_SESSION, "Close Session", "");
    addItemToTree(item, MENU_EXIT, "Close Window", "");
    tree->expandAll();
    tree->resizeColumnToContents(0);
    tree->setMinimumHeight(200);
    this->ui->le_keysh_shortcut->installEventFilter(this);
    ui->le_keysh_shortcut->setDisabled(true);
    /*************************************************************************************/

    // resize to minimum needed dimension
    this->resize(0, 0);

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
        for (int i=0; i<keyseq.count(); i++)
            shkey_entered[i] = keyseq[i];
        shkey_len = 0;
    }
}

void GuiPreferencesWindow::keysh_saveShortcutChange(QTreeWidgetItem *item)
{
    int menu_ind;
    menu_ind = !item ? -1 : item->type() - QTreeWidgetItem::UserType;
    if (menu_ind >= 0 && menu_ind < MENU_MAX_ACTION) {
        QString newseq = QKeySequence(shkey_entered[0], shkey_entered[1],
                                      shkey_entered[2], shkey_entered[3]).toString();
        QString oldseq = item->text(1);
        if (oldseq != newseq) {
            item->setText(1, newseq);
            QFont f = item->font(1);
            f.setBold(newseq != mainWindow->menuCommonActions[menu_ind]->shortcut());
            item->setFont(1, f);
            shkey_changed = true;
        }
    }
}

void GuiPreferencesWindow::on_buttonBox_accepted()
{
    bool is_config_changed = false;
    QTreeWidgetItem *root, *ch;

    /******************** Keyboard Shortcuts Tab ****************************************/
    keysh_saveShortcutChange(ui->tree_keysh->currentItem());
    if (!shkey_changed)
        goto keyboard_shortcut_done;
    root = ui->tree_keysh->invisibleRootItem();
    for(int i = 0; i < root->childCount(); i++) {
        QTreeWidgetItem *item = root->child(i);
        if (item == shkey_root_custom_saved_session)
            continue;
        for(int j = 0; j < item->childCount(); j++) {
            ch = item->child(j);
            int menu_ind = ch->type() - QTreeWidgetItem::UserType;
            if (menu_ind < 0 || menu_ind >= MENU_MAX_ACTION)
                continue;
            QKeySequence newseq = ch->text(1);
            if (newseq != mainWindow->menuCommonActions[menu_ind]->shortcut()) {
                // we have a new shortcut configured
                mainWindow->menuCommonActions[menu_ind]->setShortcut(newseq);
                mainWindow->menuCommonShortcuts[menu_ind]->setKey(newseq);
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
    for(int i = 0; i < shkey_root_custom_saved_session->childCount()-1; i++) {
        QTreeWidgetItem *ch = shkey_root_custom_saved_session->child(i);
        int menu_ind = ch->type() - QTreeWidgetItem::UserType;
        QString session = ch->data(0, QTreeWidgetItem::UserType).toString();
        int opentypemode = ch->data(0, QTreeWidgetItem::UserType+1).toInt();
        if (menu_ind != MENU_CUSTOM_OPEN_SAVED_SESSION || ch->text(1).isEmpty() ||
            session.isEmpty() || (opentypemode < 1 && opentypemode > 3))
            continue;
        QKeySequence newseq = ch->text(1);
        auto saved_list = qutty_config.menu_action_list.equal_range(menu_ind);
        for (auto it = saved_list.first; it != saved_list.second; ++it) {
            if (newseq == it->second.shortcut) {
                qutty_config.menu_action_list.erase(it);
                break;
            }
        }
        QtMenuActionConfig action(MENU_CUSTOM_OPEN_SAVED_SESSION, newseq, "", 1,
                                  session, opentypemode);
        qutty_config.menu_action_list.insert(std::make_pair(MENU_CUSTOM_OPEN_SAVED_SESSION,
                                                action));
        is_config_changed = true;
    }
    /*************************************************************************************/

keyboard_shortcut_done:

    if (is_config_changed)
        qutty_config.saveConfig();

    return;
}

void GuiPreferencesWindow::on_btn_keysh_clear_clicked()
{
    ui->le_keysh_shortcut->clear();
    shkey_len = 0;
    memset(shkey_entered, 0, sizeof(shkey_entered));
}

void GuiPreferencesWindow::on_keysh_custom_saved_session_shortcut_create()
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
        sesslist.append(it->first.c_str());
    }
    session = QInputDialog::getItem(this, "Select session",
                        "Select a session to create shortcut for",
                        sesslist, 0, false, &ok);
    if (!ok || session == sesslist[0])
        return;
    opentypelist.append(tr("--Select open mode--"));
    opentypelist.append(tr("Open in a New Tab"));
    opentypelist.append(tr("Open in Horizontal Split"));
    opentypelist.append(tr("Open in Vertiocal Split"));
    opentype = QInputDialog::getItem(this, tr("Select open mode"),
                                     tr("How do you want to open the session"),
                                     opentypelist, 0, false, &ok);
    if (!ok || opentype == opentypelist[0])
        return;

    opentypeind = opentype==opentypelist[1] ? 1 :
                  opentype==opentypelist[1] ? 2 : 3;
    QString opentypestr = opentypeind==1 ? "New Tab" :
                          opentypeind==1 ? "Horizontal Split" :
                                           "Vertical Split";
    QTreeWidgetItem *item = new QTreeWidgetItem(QTreeWidgetItem::UserType + MENU_CUSTOM_OPEN_SAVED_SESSION);
    item->setText(0, "Open '" + session + "' in " + opentypestr);
    item->setToolTip(0, "Custom keyboard shortcut to open saved session '" + session +
                     "' in " + opentypestr);
    item->setData(0, QTreeWidgetItem::UserType, session);
    item->setData(0, QTreeWidgetItem::UserType+1, opentypeind);
    shkey_root_custom_saved_session->insertChild(shkey_root_custom_saved_session->childCount()-1,
                                                item);
}
