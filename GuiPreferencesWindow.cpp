/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QDebug>
#include <QPushButton>
#include "GuiPreferencesWindow.h"
#include "ui_GuiPreferencesWindow.h"

GuiPreferencesWindow::GuiPreferencesWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GuiPreferencesWindow)
{
    ui->setupUi(this);

    QTreeWidgetItem *item = new QTreeWidgetItem(this->ui->tree_keysh, QStringList("12345"));
    this->ui->tree_keysh->setItemWidget(item, 1,
                                        new QPushButton("hello"));

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
