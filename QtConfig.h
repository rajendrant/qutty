#ifndef QTCONFIG_H
#define QTCONFIG_H

extern "C" {
#include "putty.h"
}
#include <QIODevice>
#include <QKeySequence>
#include <QString>
#include <map>
#include <string>
#include <stddef.h>

using namespace std;

class QtMenuActionConfig {
public:
    uint32_t id;
    QKeySequence shortcut;
    QString name;
    QString str_data;
    uint32_t int_data;

    QtMenuActionConfig(uint32_t _id, QKeySequence &k, QString n="",
                       QString s="", uint32_t i=0)
        : id(_id), shortcut(k), name(n), str_data(s), int_data(i)
    { }
};

class QtConfig : public QObject {

    Q_OBJECT

public:
    map<string, string> ssh_host_keys;
    map<string, Config> config_list;
    multimap<uint32_t, QtMenuActionConfig> menu_action_list;

    QtConfig();

    bool restoreConfig();
    bool saveConfig();

signals:
    void savedSessionsChanged();

private:
    int readFromXML(QIODevice *device);
    int writeToXML(QIODevice *device);
    bool restoreFromPuttyWinRegistry();
};

// all global config is here
extern QtConfig qutty_config;

#define QUTTY_DEFAULT_CONFIG_SETTINGS "Default Settings"

int initConfigDefaults(Config *cfg);

#endif // QTCONFIG_H
