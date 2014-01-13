/*
 * Copyright (C) 2013 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include <QScrollBar>
#include "GuiMainWindow.h"
#include "GuiTerminalWindow.h"
#include "GuiFindToolBar.h"

GuiFindToolBar::GuiFindToolBar(GuiMainWindow *p)
    : QToolBar(p),
      mainWnd(p)
{
    searchedText = new QLineEdit();
    findTextFlag = false;
    currentRow = -1;
    currentCol = -1;
    pageStartPosition = 0;
    QToolButton *b;
    currentSearchedText = "";
    addWidget(searchedText);

    b = new QToolButton(this);
    b->setText("Up");
    connect(b, SIGNAL(clicked()), this, SLOT(on_findUp()));
    addWidget(b);

    b = new QToolButton(this);
    b->setText("Down");
    connect(b, SIGNAL(clicked()), this, SLOT(on_findDown()));
    addWidget(b);

    b = new QToolButton(this);
    b->setIcon(QIcon(":/images/cog_alt_16x16.png"));
    b->setMenu(p->menuGetMenuById(MENU_FIND_OPTIONS));
    b->setPopupMode(QToolButton::InstantPopup);
    addWidget(b);

    b = new QToolButton(this);
    b->setIcon(QIcon(":/images/x_14x14.png"));
    connect(b, SIGNAL(clicked()), this, SLOT(on_findClose()));
    addWidget(b);

    setIconSize(QSize(16, 16));
    setMovable(false);
    setAutoFillBackground(true);
    adjustSize();

    searchedText->setFocus();
}

void GuiMainWindow::contextMenuFind()
{
    if (!this->getCurrentTerminal())
        return;

    if (!findToolBar) {
        findToolBar = new GuiFindToolBar(this);
        findToolBar->show();

        GuiTerminalWindow *t = getCurrentTerminal();
        findToolBar->move(t->viewport()->width() - findToolBar->width(),
                          t->mapTo(this, QPoint(0,0)).y());

        menuGetActionById(MENU_FIND_NEXT)->setEnabled(true);
        menuGetActionById(MENU_FIND_PREVIOUS)->setEnabled(true);
    } else {
        findToolBar->on_findClose();
    }
}

void GuiMainWindow::contextMenuFindNext()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    if (!findToolBar)
        return;
    findToolBar->on_findUp();
}

void GuiMainWindow::contextMenuFindPrevious()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action)
        return;
    if (!findToolBar)
        return;
    findToolBar->on_findDown();
}

QString GuiFindToolBar::getSearchedText()
{
    return searchedText->text();
}

extern "C" termline *decompressline(unsigned char *data, int *bytes_used);

void GuiFindToolBar::on_findUp()
{
    termline *line;
    tree234 *whichtree;
    unsigned long tchar;
    findTextFlag = true;
    GuiTerminalWindow *gterm = mainWnd->getCurrentTerminal();
    Terminal *term;
    QScrollBar *scrollbar;
    QString str = "";
    int tempCol, tempRow;
    int tempStartPosition = 0;

    if (!gterm)
        return;

    term = gterm->term;
    scrollbar = gterm->verticalScrollBar();

    if(getSearchedText() == "")
    {
        findTextFlag = false;
        gterm->viewport()->repaint();
        return;
    }
    tempCol = currentCol;
    tempRow = currentRow;

    currentCol -= currentSearchedText.length();

    if(getSearchedText().compare(currentSearchedText, Qt::CaseInsensitive))
    {
        pageStartPosition = scrollbar->value();
        currentSearchedText = getSearchedText();
        if(currentCol < 0)
            currentCol = term->cols-1;
        whichtree = NULL;
        qDebug() << "Total : " << scrollbar->maximum()+term->rows;
        qDebug() << "Value : " << scrollbar->value();
    }

    if(currentRow < 0)
        currentRow = scrollbar->value() + term->rows-1;

    while(1)
    {
        str = "";
        if(currentRow < 0)
        {
            currentRow = scrollbar->maximum() + term->rows-1;
        }
        if(count234(term->scrollback) > currentRow)
        {
            whichtree = term->scrollback;
            unsigned char *cline = (unsigned char*)index234(whichtree, currentRow);
            line = (termline*) decompressline(cline, NULL);
        }
        else
        {
            whichtree =term->screen;
            line = (termline*)index234(whichtree, currentRow - count234(term->scrollback));
        }
        for(int i = 0; i < line->size; i++)
        {
            tchar = line->chars[i].chr;
            switch (tchar & CSET_MASK)
            {
              case CSET_ASCII:
            tchar = term->ucsdata->unitab_line[tchar & 0xFF];
            break;
              case CSET_LINEDRW:
            tchar = term->ucsdata->unitab_xterm[tchar & 0xFF];
            break;
              case CSET_SCOACS:
            tchar = term->ucsdata->unitab_scoacs[tchar&0xFF];
            break;
            }
            str.append((char)tchar);
        }
        if(currentCol >= 0 && (currentCol = str.lastIndexOf(currentSearchedText, currentCol, Qt::CaseInsensitive)) >= 0)
        {
            if(pageStartPosition != scrollbar->value())
                gterm->setScrollBar(scrollbar->maximum()+term->rows, pageStartPosition, term->rows);
            else if(currentRow < scrollbar->value() || currentRow > (scrollbar->value() + term->rows))
            {
                gterm->setScrollBar(scrollbar->maximum()+term->rows, currentRow, term->rows);
                pageStartPosition = scrollbar->value();
            }
            currentSearchedText = str.mid(currentCol, currentSearchedText.length());
            tempStartPosition = pageStartPosition;
            break;
        }
        tempStartPosition++;
        if(tempStartPosition > (scrollbar->maximum()+term->rows))
        {
            findTextFlag = false;
            currentCol = tempCol;
            currentRow = tempRow;
            gterm->viewport()->repaint();
            return;
        }
        currentRow--;
        currentCol = gterm->term->cols-1;
    }

    gterm->viewport()->repaint();
}

void GuiFindToolBar::on_findDown()
{
    termline *line;
    tree234 *whichtree;
    unsigned long tchar;
    GuiTerminalWindow *gterm = mainWnd->getCurrentTerminal();
    Terminal *term;
    QScrollBar *scrollbar;
    QString str = "";
    int tempStartPosition = 0;
    int tempCol, tempRow;

    if (!gterm)
        return;

    term = gterm->term;
    scrollbar = gterm->verticalScrollBar();

    findTextFlag = true;
    if(getSearchedText() == "")
    {
        findTextFlag = false;
        gterm->viewport()->repaint();
        return;
    }
    tempCol = currentCol;
    tempRow = currentRow;
    currentCol = currentCol + currentSearchedText.length();

    if(getSearchedText().compare(currentSearchedText, Qt::CaseInsensitive))
    {
        pageStartPosition = scrollbar->value();
        currentSearchedText = getSearchedText();
        if(currentCol < 0)
            currentCol = 0;
        whichtree = NULL;
    }

    if(currentRow < 0)
        currentRow = scrollbar->value();

    while(1)
    {
        str = "";

        if(currentRow >= scrollbar->maximum()+term->rows)
        {
            currentRow = 0;
        }
        if(count234(term->scrollback) > currentRow)
        {
            whichtree = term->scrollback;
            unsigned char *cline = (unsigned char*)index234(whichtree, currentRow);
            line = (termline*) decompressline(cline, NULL);
        }
        else
        {
            whichtree = term->screen;
            line = (termline*)index234(whichtree, abs(currentRow - count234(term->scrollback)));
        }
        for(int i = 0; i < line->size; i++)
        {
            //qDebug() << line->chars[i].chr;
            tchar = line->chars[i].chr;
            switch (tchar & CSET_MASK)
            {
              case CSET_ASCII:
            tchar = term->ucsdata->unitab_line[tchar & 0xFF];
            break;
              case CSET_LINEDRW:
            tchar = term->ucsdata->unitab_xterm[tchar & 0xFF];
            break;
              case CSET_SCOACS:
            tchar = term->ucsdata->unitab_scoacs[tchar&0xFF];
            break;
            }
            str.append((char)tchar);
        }
        if(currentCol < term->cols && (currentCol = str.indexOf(currentSearchedText, currentCol, Qt::CaseInsensitive)) >= 0)
        {
            if(pageStartPosition != scrollbar->value())
                gterm->setScrollBar(scrollbar->maximum()+term->rows, pageStartPosition, term->rows);
            else
            {
                if(currentRow < scrollbar->value() || currentRow >= (scrollbar->value() + term->rows))
                {
                    gterm->setScrollBar(scrollbar->maximum()+term->rows, currentRow, term->rows);
                    pageStartPosition = scrollbar->value();
                }
            }
            currentSearchedText = str.mid(currentCol, currentSearchedText.length());
            tempStartPosition = pageStartPosition;
            break;
        }
        tempStartPosition++;
        if(tempStartPosition > (scrollbar->maximum()+term->rows))
        {
            findTextFlag = false;
            currentCol = tempCol;
            currentRow = tempRow;
            gterm->viewport()->repaint();
            return;
        }
        currentRow++;
        currentCol = 0;
    }
    gterm->viewport()->repaint();
}

void GuiFindToolBar::on_findClose()
{
    GuiTerminalWindow *t;

    findTextFlag = false;
    mainWnd->menuGetActionById(MENU_FIND_NEXT)->setEnabled(false);
    mainWnd->menuGetActionById(MENU_FIND_PREVIOUS)->setEnabled(false);
    mainWnd->findToolBar = NULL;
    this->deleteLater();

    if (t=mainWnd->getCurrentTerminal()) {
        t->viewport()->repaint();
        t->setFocus();
    }
}
