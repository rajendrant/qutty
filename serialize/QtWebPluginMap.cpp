#include "QtWebPluginMap.h"
#include <QDataStream>
#include <QFile>
#include <QDir>

static const char *serialize_file_name = "qutty/qt_web_plugin_map.txt";

QtWebPluginMap qutty_web_plugin_map;

QtWebPluginMap::QtWebPluginMap()
{
}

QtWebPluginMap::~QtWebPluginMap()
{
}

void QtWebPluginMap::initialize()
{
    QFile file(QDir::home().filePath(serialize_file_name));
    if (file.open(QFile::ReadOnly)) {
        QDataStream stream(&file);
        stream >> hash_map;
    }
}

void QtWebPluginMap::save()
{
    QFile file(QDir::home().filePath(serialize_file_name));
    if (file.open(QFile::WriteOnly | QFile::Truncate)) {
        QDataStream stream(&file);
        stream << hash_map;
    }
}
