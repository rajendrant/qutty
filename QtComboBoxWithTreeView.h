#ifndef QTCOMBOBOXWITHTREEVIEW_H
#define QTCOMBOBOXWITHTREEVIEW_H

#include <QComboBox>
#include <QMouseEvent>
#include <QModelIndex>
#include <QAbstractItemView>
#include <QTreeView>
#include <QApplication>

/*
 * From the hack provided in below url:
 * http://www.developer.nokia.com/Community/Wiki/Implementing_QTreeView_in_QComboBox_using_Qt-_Part_2
 *
 * There are some bugs being worked on. Hopefully patch will reach Qt5
 * and we can remove this file.
 * https://bugreports.qt-project.org/browse/QTBUG-11660
 * https://codereview.qt-project.org/#change,16716
 */

class QtComboBoxWithTreeView : public QComboBox
{
    Q_OBJECT
public:
    explicit QtComboBoxWithTreeView(QWidget *parent = 0) :
        QComboBox(parent), skipNextHide(false)
    {
        QTreeView *view = new QTreeView(this);
        view->setHeaderHidden(true);
        setView(view);
        view->viewport()->installEventFilter(this);
    }

    bool eventFilter(QObject* object, QEvent* event)
    {
        if (event->type() == QEvent::MouseButtonPress && object == view()->viewport())
        {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            QModelIndex index = view()->indexAt(mouseEvent->pos());
            if (!view()->visualRect(index).contains(mouseEvent->pos()))
                skipNextHide = true;
        }
        return false;
    }

    virtual void hidePopup()
    {
        if (skipNextHide) {
            skipNextHide = false;
            /*
             * We are doing a big hack here.
             * This is done in order to expand/shrink the popup
             * as trees expand/shrink
             */
            bool old = QApplication::isEffectEnabled(Qt::UI_AnimateCombo);
            if (old)
                QApplication::setEffectEnabled(Qt::UI_AnimateCombo, false);
            QComboBox::hidePopup();
            QComboBox::showPopup();
            if (old)
                QApplication::setEffectEnabled(Qt::UI_AnimateCombo, true);
        } else
            QComboBox::hidePopup();
    }

private:
    bool skipNextHide;
};

#endif // QTCOMBOBOXWITHTREEVIEW_H
