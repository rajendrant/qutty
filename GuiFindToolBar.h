/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#ifndef GUIFINDTOOLBAR_H
#define GUIFINDTOOLBAR_H

#include <QToolBar>
#include <QLineEdit>

class GuiMainWindow;

class GuiFindToolBar : public QToolBar
{
    Q_OBJECT

    GuiMainWindow *mainWnd;
    QLineEdit *searchedText;

public:
    bool findTextFlag;
    int currentRow;
    int currentCol;
    int pageStartPosition;
    QString currentSearchedText;
    GuiFindToolBar(GuiMainWindow *p);
    virtual ~GuiFindToolBar() { }
    QString getSearchedText();


public slots:
    void on_findUp();
    void on_findDown();
    void on_findClose();
};

#endif // GUIFINDTOOLBAR_H
