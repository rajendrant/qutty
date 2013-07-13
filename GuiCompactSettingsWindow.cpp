#include <QDialogButtonBox>
#include <QLabel>
#include <QHBoxLayout>
#include "GuiCompactSettingsWindow.h"
#include "GuiMainWindow.h"

GuiCompactSettingsWindow::GuiCompactSettingsWindow(QWidget *parent, GuiBase::SplitType openmode)
    : QDialog(parent)
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

    le_hostname = new QLineEdit(this);
    le_hostname->setMinimumWidth(500);

    cb_connection_type = new QComboBox(this);
    cb_connection_type->setMaximumWidth(100);
    cb_connection_type->addItem("Telnet");
    cb_connection_type->addItem("SSH");

    cb_session_list = new QComboBox(this);
    cb_session_list->setMinimumWidth(500);
    map<string, Config>::iterator it = qutty_config.config_list.begin();
    cfg = &(it->second);
    le_hostname->setText(QString(cfg->host));
    if(cfg->protocol == PROT_TELNET)
        cb_connection_type->setCurrentIndex(0);
    else
        cb_connection_type->setCurrentIndex(1);
    while(it != qutty_config.config_list.end())
    {
        cfg = &(it->second);
        cb_session_list->addItem(QString(cfg->config_name));
        it++;
    }

    connect(cb_session_list, SIGNAL(activated(int)), this, SLOT(on_cb_session_list_activated(int)));

    QGridLayout *layout = new QGridLayout(this);

    layout->addWidget(new QLabel("Hostname : ", this));
    layout->addWidget(le_hostname);

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
    char configName[100];
    if (le_hostname->text() == "" &&
        cb_session_list->currentText() == QUTTY_DEFAULT_CONFIG_SETTINGS)
        return;
    qstring_to_char(configName, cb_session_list->currentText(), sizeof(configName));
    if (qutty_config.config_list.find(configName) == qutty_config.config_list.end())
        return;
    cfg = qutty_config.config_list[configName];
    qstring_to_char(cfg.host, le_hostname->text(), sizeof(cfg.host));

    if(cb_connection_type->currentText() == "SSH")
        cfg.protocol = PROT_SSH;
    else
        cfg.protocol = PROT_TELNET;
    emit signal_on_open(cfg, openMode);
    this->close();
    this->deleteLater();
}

void GuiCompactSettingsWindow::on_details_clicked()
{
    emit signal_on_detail(openMode);
    this->close();
    this->deleteLater();
}

void GuiCompactSettingsWindow::on_cb_session_list_activated(int n)
{
    char configName[100];
    Config *cfg;
    qstring_to_char(configName, cb_session_list->itemText(n), sizeof(configName));
    map<string, Config>::iterator it = qutty_config.config_list.find(configName);

    if(it != qutty_config.config_list.end())
    {
        cfg = &(it->second);
        le_hostname->setText(QString(cfg->host));
        if(cfg->protocol == PROT_TELNET)
            cb_connection_type->setCurrentIndex(0);
        else
            cb_connection_type->setCurrentIndex(1);
    }
}
