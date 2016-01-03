#ifndef QTSESSIONTREEMODEL_H
#define QTSESSIONTREEMODEL_H

#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include "QtSessionTreeItem.h"
#include "QtConfig.h"

using namespace std;

class QtSessionTreeModel : public QAbstractItemModel {
  Q_OBJECT
 public:
  explicit QtSessionTreeModel(QObject *parent, map<QString, Config> &config_list);
  ~QtSessionTreeModel();

  QVariant data(const QModelIndex &index, int role) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
  QModelIndex parent(const QModelIndex &index) const;
  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;

  QModelIndex findIndexForSessionName(QString fullsessname) const;

 private:
  QtSessionTreeItem *rootItem;
};

class QtSessionTreeItemDelegate : public QStyledItemDelegate {
  QString displayText(const QVariant &value, const QLocale & /*locale*/) const {
    QString s = value.toString();
    int i = s.lastIndexOf('/');
    if (i != -1) return s.right(s.length() - i - 1);
    return s;
  }
};

#endif  // QTSESSIONTREEMODEL_H
