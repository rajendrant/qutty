#ifndef QTCONFIG_H
#define QTCONFIG_H

#include "putty.h"
#include <QIODevice>
#include <map>
#include <string>

using namespace std;

class QtConfig {
public:
    map<string, string> ssh_host_keys;
    map<string, Config> config_list;

    QtConfig();

    int readFromXML(QIODevice *device);
    int writeToXML(QIODevice *device);

    bool restoreConfig();
    bool saveConfig();
};

// all global config is here
extern QtConfig qutty_config;

#define QUTTY_DEFAULT_CONFIG_SETTINGS "Default Settings"

int initConfigDefaults(Config *cfg);

#endif // QTCONFIG_H
