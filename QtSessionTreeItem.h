#ifndef QTSESSIONTREEITEM_H
#define QTSESSIONTREEITEM_H

#include <QString>

class QtSessionTreeItem {
 public:
  explicit QtSessionTreeItem(const QString &sessionName, QtSessionTreeItem *parent)
      : sessionName(sessionName), parentItem(parent) {}

  ~QtSessionTreeItem() { qDeleteAll(childItems); }

  void appendChild(QtSessionTreeItem *child) { childItems.append(child); }

  QtSessionTreeItem *child(int row) { return childItems.value(row); }
  int childCount() const { return childItems.count(); }

  int row() const {
    if (parentItem) return parentItem->childItems.indexOf(const_cast<QtSessionTreeItem *>(this));
    return 0;
  }

  QtSessionTreeItem *parent() { return parentItem; }

  QString getSessionName() const { return sessionName; }

  QString getFullSessionName() const {
    if (parentItem) {
      if (parentItem->parent()) return parentItem->getFullSessionName() + "/" + sessionName;
      return sessionName;
    }
    return "";
  }

 private:
  QList<QtSessionTreeItem *> childItems;
  QString sessionName;
  QtSessionTreeItem *parentItem;
};

#endif  // QTSESSIONTREEITEM_H
