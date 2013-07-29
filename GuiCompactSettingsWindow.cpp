#include <QDialogButtonBox>
#include <QLabel>
#include <QHBoxLayout>
#include "GuiCompactSettingsWindow.h"
#include "GuiMainWindow.h"
#include "QtSessionTreeModel.h"
#include "QtCompleterWithAdvancedCompletion.h"
#include "serialize/QtMRUSessionList.h"
#include <QStringList>

GuiCompactSettingsWindow::GuiCompactSettingsWindow(QWidget *parent, GuiBase::SplitType openmode)
    : QDialog(parent),
      session_list_model(NULL)
{
    openMode = openmode;

    Config* cfg;

    QPushButton *details = new QPushButton(tr("Details"));
    connect(details, SIGNAL(clicked()), this, SLOT(on_details_clicked()));

    QDialogButtonBox *btn_box = new QDialogButtonBox(QDialogButtonBox::Cancel);

    btn_box->addButton(tr("Open"), QDialogButtonBox::AcceptRole);
    connect(btn_box, SIGNAL(accepted()), this, SLOT(on_open_clicked()));
    connect(btn_box, SIGNAL(rejected()), this, SLOT(on_close_clicked()));
    connect(this, SIGNAL(rejected()), this, SLOT(on_close_clicked()));

    cb_hostname = new QComboBox(this);
    cb_hostname->setMinimumWidth(500);

    QStringList completions;
    for(auto it = qutty_mru_sesslist.mru_list.begin();
        it != qutty_mru_sesslist.mru_list.end();
        ++it) {
        if (it->second[0] == '\0')
            continue;
        // in 'hostname|sessname' format
        completions << it->second + "|" + it->first;
    }
    hostname_completer = new QtCompleterWithAdvancedCompletion(cb_hostname);
    hostname_completer->setModel(completions);
    hostname_completer->popup()->setItemDelegate(new QtHostNameCompleterItemDelegate);
    connect(hostname_completer, SIGNAL(activated(QString)), this, SLOT(on_hostname_completion_activated(QString)));
    cb_hostname->setItemDelegate(new QtHostNameCompleterItemDelegate);
    connect(cb_hostname, SIGNAL(activated(QString)), SLOT(on_cb_hostname_activated(QString)));

    cb_session_list = new QtComboBoxWithTreeView(this);
    cb_session_list->setItemDelegate(new QtSessionTreeItemDelegate);
    session_list_model = new QtSessionTreeModel(this, qutty_config.config_list);
    cb_session_list->setModel(session_list_model);
    cb_session_list->setMaxVisibleItems(25);

    cb_connection_type = new QComboBox(this);
    cb_connection_type->setMaximumWidth(100);
    cb_connection_type->addItem("Telnet");
    cb_connection_type->addItem("SSH");

    if (qutty_mru_sesslist.mru_list.size() > 0 &&
        qutty_config.config_list.find(qutty_mru_sesslist.mru_list[0].first) != qutty_config.config_list.end()) {
        QString sessname = qutty_mru_sesslist.mru_list[0].first;
        cfg = &qutty_config.config_list[sessname];
        cb_session_list->setCurrentIndex(cb_session_list->findText(sessname));
        hostname_completer->setText(QString(qutty_mru_sesslist.mru_list[0].second));
        if(cfg->protocol == PROT_TELNET)
            cb_connection_type->setCurrentIndex(0);
        else
            cb_connection_type->setCurrentIndex(1);
    } else if(qutty_config.config_list.find(QUTTY_DEFAULT_CONFIG_SETTINGS) != qutty_config.config_list.end())
    {
        cfg = &qutty_config.config_list[QUTTY_DEFAULT_CONFIG_SETTINGS];
        cb_session_list->setCurrentIndex(cb_session_list->findText(QUTTY_DEFAULT_CONFIG_SETTINGS));
        hostname_completer->setText(QString(cfg->host));
        if(cfg->protocol == PROT_TELNET)
            cb_connection_type->setCurrentIndex(0);
        else
            cb_connection_type->setCurrentIndex(1);
    }

    connect(cb_session_list, SIGNAL(activated(int)), this, SLOT(on_cb_session_list_activated(int)));

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(new QLabel("Hostname : ", this));
    layout->addWidget(cb_hostname);

    layout->addWidget(new QLabel("Session profiles : ", this));
    layout->addWidget(cb_session_list);

    layout->addWidget(new QLabel("Connection type : ", this));
    layout->addWidget(cb_connection_type);

    QHBoxLayout *hlayout = new QHBoxLayout();
    hlayout->addWidget(details);
    hlayout->addWidget(btn_box, 1);

    layout->addLayout(hlayout, 10, 0, 1, 1);

    setLayout(layout);
}

void GuiCompactSettingsWindow::on_close_clicked()
{
    emit signal_on_close();
    this->close();
    this->deleteLater();
}

void GuiCompactSettingsWindow::on_open_clicked()
{
    Config cfg;
    QString configName;
    if (cb_hostname->currentText() == "" &&
        cb_session_list->currentText() == QUTTY_DEFAULT_CONFIG_SETTINGS)
        return;
    configName = cb_session_list->currentText();
    if (qutty_config.config_list.find(configName) == qutty_config.config_list.end())
        return;
    cfg = qutty_config.config_list[configName];
    qstring_to_char(cfg.host, cb_hostname->currentText(), sizeof(cfg.host));

    if(cb_connection_type->currentText() == "SSH")
        cfg.protocol = PROT_SSH;
    else
        cfg.protocol = PROT_TELNET;

    chkUnsupportedConfigs(cfg);

    emit signal_on_open(cfg, openMode);
    this->close();
    this->deleteLater();
}

void GuiCompactSettingsWindow::on_details_clicked()
{
    Config cfg;
    QString configName;

    configName = cb_session_list->currentText();
    if (qutty_config.config_list.find(configName) == qutty_config.config_list.end())
        return;
    cfg = qutty_config.config_list[configName];
    emit signal_on_detail(cfg, openMode);
    this->close();
    this->deleteLater();
}

void GuiCompactSettingsWindow::on_cb_session_list_activated(int n)
{
    QString configName;
    Config *cfg;
    configName = cb_session_list->currentText();
    map<QString, Config>::iterator it = qutty_config.config_list.find(configName);

    if(it != qutty_config.config_list.end())
    {
        cfg = &(it->second);
        if(cfg->host[0] != '\0')
            hostname_completer->setText(QString(cfg->host));
        if(cfg->protocol == PROT_TELNET)
            cb_connection_type->setCurrentIndex(0);
        else
            cb_connection_type->setCurrentIndex(1);
    }
}

void GuiCompactSettingsWindow::on_cb_hostname_activated(QString str)
{
    on_hostname_completion_activated(str);
    hostname_completer->popup()->hide();
}

void GuiCompactSettingsWindow::on_hostname_completion_activated(QString str)
{
    QStringList split = str.split('|');
    if (split.length() > 1) {
        hostname_completer->setText(split[0]);

        /*
         * Based on the suggestion/technique from below url:
         * http://www.qtcentre.org/threads/14699-QCombobox-with-QTreeView-QTreeWidget
         */
        bool old = QApplication::isEffectEnabled(Qt::UI_AnimateCombo);
        if (old)
            QApplication::setEffectEnabled(Qt::UI_AnimateCombo, false);

        QString fullsessname = split[1];
        QAbstractItemView *treeview = cb_session_list->view();
        QModelIndex m_index = session_list_model->findIndexForSessionName(fullsessname);
        treeview->setCurrentIndex(m_index.parent());
        cb_session_list->setRootModelIndex(treeview->currentIndex());
        cb_session_list->setCurrentIndex(m_index.row());
        treeview->setCurrentIndex(QModelIndex());
        cb_session_list->setRootModelIndex(treeview->currentIndex());

        cb_session_list->showPopup();
        cb_session_list->hidePopup();
        if (old)
            QApplication::setEffectEnabled(Qt::UI_AnimateCombo, true);
    }
}
