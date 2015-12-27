#include "QtCompleterWithAdvancedCompletion.h"
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

/*
 * There are some existing bugs to support this feature in QCompleter
 * And people are actively working on it.
 * https://bugreports.qt-project.org/browse/QTBUG-7830
 * https://bugreports.qt-project.org/browse/QTBUG-3414
 * https://codereview.qt-project.org/#change,43718
 *
 * Hopefully Qt5 will have this feature soon, and we shall remove this file
 */

QtCompleterWithAdvancedCompletion::QtCompleterWithAdvancedCompletion(QLineEdit *le) :
    QObject(le),
    popuplist(new QListView),
    w(le),
    model(new QStringListModel),
    maxVisibleItems(10),
    noItemsShown(0),
    filterMode(QtCompleterWithAdvancedCompletion::ContainsWord),
    always_show_popup(false)
{
    init();

    connect(popuplist, SIGNAL(clicked(QModelIndex)),
                     this, SLOT(slot_completerComplete(QModelIndex)));
    connect(le, SIGNAL(textEdited(QString)), this, SLOT(completionSearchString(QString)));
    connect(this, SIGNAL(activated(QString)), le, SLOT(setText(QString)));
}

QtCompleterWithAdvancedCompletion::QtCompleterWithAdvancedCompletion(QComboBox *cb) :
    QObject(cb),
    popuplist(new QListView),
    w(cb),
    model(new QStringListModel),
    maxVisibleItems(10),
    noItemsShown(0),
    filterMode(QtCompleterWithAdvancedCompletion::ContainsWord),
    always_show_popup(false)
{
    init();

    cb->setEditable(true);
    cb->setAutoCompletion(false);
    cb->installEventFilter(this);

    connect(popuplist, SIGNAL(clicked(QModelIndex)),
                     this, SLOT(slot_completerComplete(QModelIndex)));
    connect(cb, SIGNAL(editTextChanged(QString)), this, SLOT(completionSearchString(QString)));
    connect(this, SIGNAL(activated(QString)), cb, SLOT(setEditText(QString)));
}

QtCompleterWithAdvancedCompletion::~QtCompleterWithAdvancedCompletion()
{
    popuplist->deleteLater();
}

void QtCompleterWithAdvancedCompletion::init()
{
    popuplist->setEditTriggers(QAbstractItemView::NoEditTriggers);
    popuplist->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popuplist->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    popuplist->setSelectionBehavior(QAbstractItemView::SelectRows);
    popuplist->setSelectionMode(QAbstractItemView::SingleSelection);

    popuplist->setParent(NULL, Qt::Popup);
    popuplist->setFocusPolicy(Qt::NoFocus);
    popuplist->setFocusProxy(w);
    popuplist->installEventFilter(this);

    popuplist->setModel(model);
    if (always_show_popup)
        popuplist->show();
}

void QtCompleterWithAdvancedCompletion::setModel(QStringList &completions)
{
    this->completions = completions;
    QComboBox *c = qobject_cast<QComboBox*>(w);
    if (c)
        c->addItems(completions);
    int itemsToShow = completions.size();
    QPoint pos = w->mapToGlobal(QPoint(0, w->height()));
    popuplist->setGeometry(pos.x(), pos.y(), w->width(), itemsToShow*w->height());
    popuplist->show();
    noItemsShown = itemsToShow;
}

bool QtCompleterWithAdvancedCompletion::eventFilter(QObject *o, QEvent *e)
{
    if (o == w) {
        switch (e->type()) {
        case QEvent::MouseButtonPress:
            is_keypress = false;
            break;
        }
        return w->eventFilter(o, e);
    }

    if (o != popuplist)
        return QObject::eventFilter(o, e);

    switch (e->type()) {
    case QEvent::KeyPress: {
        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        const int key = ke->key();
        QModelIndex curIndex = popuplist->currentIndex();

        // Handle popup navigation keys. These are hardcoded because up/down might make the
        // widget do something else (lineedit cursor moves to home/end on mac, for instance)
        switch (key) {
        case Qt::Key_End:
        case Qt::Key_Home:
            if (ke->modifiers() & Qt::ControlModifier)
                return false;
            break;

        case Qt::Key_Up:
            if (!curIndex.isValid()) {
                int rowCount = popuplist->model()->rowCount();
                QModelIndex lastIndex = popuplist->model()->index(rowCount - 1, 0);
                popuplist->setCurrentIndex(lastIndex);
                return true;
            } else if (curIndex.row() == 0) {
                popuplist->setCurrentIndex(QModelIndex());
                return true;
            }
            return false;

        case Qt::Key_Down:
            if (!curIndex.isValid()) {
                QModelIndex firstIndex = popuplist->model()->index(0, 0);
                popuplist->setCurrentIndex(firstIndex);
                return true;
            } else if (curIndex.row() == popuplist->model()->rowCount() - 1) {
                popuplist->setCurrentIndex(QModelIndex());
                return true;
            }
            return false;

        case Qt::Key_PageUp:
        case Qt::Key_PageDown:
            return false;
        }

        (static_cast<QObject *>(w))->event(ke);
        if (e->isAccepted()) {
            if (!w->hasFocus())
                popuplist->hide();
            return true;
        }

        // default implementation for keys not handled by the widget when popup is open
        switch (key) {
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Tab:
            popuplist->hide();
            if (curIndex.isValid())
                slot_completerComplete(curIndex);
            break;

        case Qt::Key_F4:
            if (ke->modifiers() & Qt::AltModifier)
                popuplist->hide();
            break;

        case Qt::Key_Backtab:
        case Qt::Key_Escape:
            popuplist->hide();
            emit deactivated();
            break;
        }
        return true;
    }
    case QEvent::MouseButtonPress: {
        if (!popuplist->underMouse()) {
            popuplist->hide();
            return true;
        }
        return false;
    }
    }

    return false;
}

void QtCompleterWithAdvancedCompletion::slot_completerComplete(QModelIndex index)
{
    is_keypress = false;
    if (index.isValid()) {
        emit activated(index.data().toString());
    }
    popuplist->hide();
}

void QtCompleterWithAdvancedCompletion::completionSearchString(QString str)
{
    if (!always_show_popup && (!w || str.isEmpty())) {
        popuplist->hide();
        return;
    }
    if (!always_show_popup && !is_keypress) {
        is_keypress = true;
        popuplist->hide();
        return;
    }
    QStringList tmp;
    int noitems = maxVisibleItems;
    for(int i=0; i<completions.size() && noitems; i++) {
        switch (filterMode) {
        case QtCompleterWithAdvancedCompletion::StartsWith:
            if (completions[i].startsWith(str)) {
                tmp << completions[i];
                noitems--;
            }
            break;
        case QtCompleterWithAdvancedCompletion::Contains:
            if (completions[i].contains(str)) {
                tmp << completions[i];
                noitems--;
            }
            break;
        case QtCompleterWithAdvancedCompletion::ContainsWord:
            if (completions[i].contains(str)) {
                tmp << completions[i];
                noitems--;
            }
            break;
        }
    }
    model->setStringList(tmp);
    int itemsToShow = qMin(tmp.length(), maxVisibleItems);
    if (always_show_popup || popuplist->isHidden() || noItemsShown != itemsToShow) {
        QPoint pos = w->mapToGlobal(QPoint(0, w->height()));
        popuplist->setGeometry(pos.x(), pos.y(), w->width(), itemsToShow*w->height());
        popuplist->show();
        noItemsShown = itemsToShow;
    }
    if (always_show_popup && !popuplist->currentIndex().isValid())
        popuplist->setCurrentIndex(popuplist->model()->index(0, 0));
}

void QtCompleterWithAdvancedCompletion::setText(QString str)
{
    is_keypress = false;
    QComboBox *c = qobject_cast<QComboBox*>(w);
    if (c)
        c->setEditText(str);
    QLineEdit *le = qobject_cast<QLineEdit*>(w);
    if (le)
        le->setText(str);
}
