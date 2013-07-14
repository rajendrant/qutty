#ifndef QTMRUSESSIONLIST_H
#define QTMRUSESSIONLIST_H

#include <QVector>
#include <QMap>
#include <QPair>
#include <QString>

class QtMRUSessionList
{
public:
    QtMRUSessionList() { }
    void initialize();
    void insertSession(QString &sessname, QString &hostname);

    QVector<QPair<QString, QString> > mru_list;

private:
    void save();
};

extern QtMRUSessionList qutty_mru_sesslist;

#endif // QTMRUSESSIONLIST_H
