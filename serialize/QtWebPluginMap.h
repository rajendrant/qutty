#ifndef QTWEBPLUGINMAP_H
#define QTWEBPLUGINMAP_H

#include <QString>
#include <QHash>
#include <QVariantList>

class QtWebPluginMap {
 public:
  QtWebPluginMap();
  ~QtWebPluginMap();
  void initialize();
  void save();

  QHash<QString, QVariantList> hash_map;
};

extern QtWebPluginMap qutty_web_plugin_map;

#endif  // QTWEBPLUGINMAP_H
