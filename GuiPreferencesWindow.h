#ifndef GUIPREFERENCESWINDOW_H
#define GUIPREFERENCESWINDOW_H

#include <QDialog>

namespace Ui {
class GuiPreferencesWindow;
}

class GuiPreferencesWindow : public QDialog
{
    Q_OBJECT
    
public:
    explicit GuiPreferencesWindow(QWidget *parent = 0);
    ~GuiPreferencesWindow();
    
private slots:
    void slot_GuiPreferencesWindow_rejected();

private:
    Ui::GuiPreferencesWindow *ui;
};

#endif // GUIPREFERENCESWINDOW_H
