#ifndef QTCONFIG_H
#define QTCONFIG_H

extern "C" {
#include "putty.h"
}
#include <QIODevice>
#include <QKeySequence>
#include <QString>
#include <QPoint>
#include <QSize>
#include <map>
#include <vector>
#include <string>
#include <stddef.h>
#include <QFile>


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

typedef struct qutty_mainwindow_settings_t__ {
    QSize size;
    QPoint pos;
    int state;
    int flag;
    bool menubar_visible;
    bool titlebar_tabs;
} qutty_mainwindow_settings_t;

class QtConfig : public QObject {

    Q_OBJECT

public:
    map<string, string> ssh_host_keys;
    map<QString, Config> config_list;
    map<uint32_t, QtMenuActionConfig> menu_action_list;
    qutty_mainwindow_settings_t mainwindow;

    QtConfig();

    bool restoreConfig();
    bool saveConfig();
    void importFromFile(QFile*);
    void importFromPutty();
    void exportToFile(QFile*);

signals:
    void savedSessionsChanged();

private:
    int readFromXML(QIODevice *device);
    int writeToXML(QIODevice *device);
    bool restoreFromPuttyWinRegistry();
};

// all global config is here
extern QtConfig qutty_config;

extern vector<string> qutty_string_split(string &str, char delim);

#define QUTTY_DEFAULT_CONFIG_SETTINGS "Default Settings"
#define QUTTY_SESSION_NAME_SPLIT '/'

int initConfigDefaults(Config *cfg);

#endif // QTCONFIG_H
