#include "QtMRUSessionList.h"
#include <QDataStream>
#include <QFile>
#include <QDir>
#include "QtConfig.h"

static const char *serialize_file_name = "qutty/mru_sesslist.txt";

QtMRUSessionList qutty_mru_sesslist;

void QtMRUSessionList::initialize()
{
    QFile file(QDir::home().filePath(serialize_file_name));
    if (file.open(QFile::ReadOnly)) {
        QDataStream stream(&file);
        stream >> mru_list;
    } else {
        for(auto it = qutty_config.config_list.begin();
            it != qutty_config.config_list.end();
            ++it) {
            QPair<QString, QString> e(QString::fromStdString(it->first), it->second.host);
            mru_list.append(e);
        }
        save();
    }
}

void QtMRUSessionList::insertSession(QString &sessname, QString &hostname)
{
    QPair<QString, QString> e(sessname, hostname);
    int ind = mru_list.indexOf(e);
    if (ind != -1)
        mru_list.remove(ind);
    mru_list.push_front(e);
    save();
}

void QtMRUSessionList::save()
{
    QFile file(QDir::home().filePath(serialize_file_name));
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QDataStream stream(&file);
        stream << mru_list;
    }
}
