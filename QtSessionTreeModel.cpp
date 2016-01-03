#include "QtSessionTreeModel.h"
#include "QtConfig.h"

QtSessionTreeModel::QtSessionTreeModel(QObject *parent, map<QString, Config> &config_list)
    : QAbstractItemModel(parent) {
  map<QString, QtSessionTreeItem *> folders;
  rootItem = new QtSessionTreeItem("Session Name", NULL);
  folders[""] = rootItem;

  for (std::map<QString, Config>::iterator it = config_list.begin(); it != config_list.end();
       it++) {
    QString fullsessname = it->first;
    if (folders.find(fullsessname) != folders.end()) continue;
    if (fullsessname.endsWith(QUTTY_SESSION_NAME_SPLIT)) fullsessname.chop(1);
    QStringList split = fullsessname.split(QUTTY_SESSION_NAME_SPLIT);
    QString sessname = split.back();
    QString dirname = fullsessname.mid(0, fullsessname.lastIndexOf(QUTTY_SESSION_NAME_SPLIT));
    if (dirname == fullsessname) dirname = "";
    if (folders.find(dirname) == folders.end()) {
      QtSessionTreeItem *par = rootItem;
      QString tmpdir = "";
      for (int i = 0; i < split.size() - 1; i++) {
        tmpdir += split[i];
        if (folders.find(tmpdir) == folders.end()) {
          QtSessionTreeItem *newitem = new QtSessionTreeItem(split[i], par);
          par->appendChild(newitem);
          folders[tmpdir] = newitem;
          par = newitem;
        } else
          par = folders[tmpdir];
        tmpdir += QUTTY_SESSION_NAME_SPLIT;
      }
    }
    QtSessionTreeItem *item = new QtSessionTreeItem((sessname), folders[dirname]);
    folders[dirname]->appendChild(item);
    folders[fullsessname] = item;
  }
}

QtSessionTreeModel::~QtSessionTreeModel() { delete rootItem; }

int QtSessionTreeModel::columnCount(const QModelIndex &parent) const {
  return 1;  // only 1 column - session_name
}

QModelIndex QtSessionTreeModel::findIndexForSessionName(QString fullsessname) const {
  QStringList dirname = fullsessname.split(QUTTY_SESSION_NAME_SPLIT);
  QModelIndex par;
  QModelIndex ch;
  for (auto it = dirname.begin(); it != dirname.end(); it++) {
    bool isfound = false;
    for (int r = 0; r < rowCount(par); r++) {
      ch = index(r, 0, par);
      if (!ch.isValid()) continue;
      QtSessionTreeItem *chitem = static_cast<QtSessionTreeItem *>(ch.internalPointer());
      if (chitem->getSessionName() == *it) {
        par = ch;
        isfound = true;
        break;
      }
    }
    if (!isfound) {
      return QModelIndex();
    }
  }
  return ch;
}

QVariant QtSessionTreeModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || role != Qt::DisplayRole) return QVariant();

  QtSessionTreeItem *item = static_cast<QtSessionTreeItem *>(index.internalPointer());

  return item->getFullSessionName();
}

Qt::ItemFlags QtSessionTreeModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) return 0;

  return QAbstractItemModel::flags(index);
}

QVariant QtSessionTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) return rootItem->getSessionName();

  return QVariant();
}

QModelIndex QtSessionTreeModel::index(int row, int column, const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent)) return QModelIndex();

  QtSessionTreeItem *parentItem;

  if (!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<QtSessionTreeItem *>(parent.internalPointer());

  QtSessionTreeItem *childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);
  else
    return QModelIndex();
}

QModelIndex QtSessionTreeModel::parent(const QModelIndex &index) const {
  if (!index.isValid()) return QModelIndex();

  QtSessionTreeItem *childItem = static_cast<QtSessionTreeItem *>(index.internalPointer());
  QtSessionTreeItem *parentItem = childItem->parent();

  if (parentItem == rootItem) return QModelIndex();

  return createIndex(parentItem->row(), 0, parentItem);
}

int QtSessionTreeModel::rowCount(const QModelIndex &parent) const {
  QtSessionTreeItem *parentItem;
  if (parent.column() > 0) return 0;

  if (!parent.isValid())
    parentItem = rootItem;
  else
    parentItem = static_cast<QtSessionTreeItem *>(parent.internalPointer());

  return parentItem->childCount();
}
