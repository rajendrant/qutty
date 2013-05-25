/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

extern "C" {
#include "putty.h"
#include "ssh.h"
}
#include "GuiTerminalWindow.h"
#include <QMessageBox>
#include <QtConfig.h>

#define APPNAME "PuTTY"

/*
 * Ask whether the selected algorithm is acceptable (since it was
 * below the configured 'warn' threshold).
 */
int askalg(void *frontend, const char *algtype, const char *algname,
       void (*callback)(void *ctx, int result), void *ctx)
{
    assert(frontend);
    GuiTerminalWindow *f = static_cast<GuiTerminalWindow*>(frontend);
    QString msg = 	QString("The first " + QString(algtype) + " supported by the server\n"
                            "is "+ QString(algname) +", which is below the configured\n"
                            "warning threshold.\n"
                            "Do you want to continue with this connection?\n");
    switch (QMessageBox::warning(f->getMainWindow(), QString(APPNAME " Security Alert"),
                         msg,
                         QMessageBox::Yes | QMessageBox::No,
                         QMessageBox::No)) {
    case QMessageBox::Yes:      return 2;
    case QMessageBox::No:       return 1;
    default:                    return 0;
    }
}

/*
 * Ask whether to wipe a session log file before writing to it.
 * Returns 2 for wipe, 1 for append, 0 for cancel (don't log).
 */
int askappend(void *frontend, Filename filename,
          void (*callback)(void *ctx, int result), void *ctx)
{
    assert(frontend);
    GuiTerminalWindow *f = static_cast<GuiTerminalWindow*>(frontend);
    QString msg = 	QString("The session log file \"") + QString(filename.path)
            + QString("\" already exists.\n"
            "You can overwrite it with a new session log,\n"
            "append your session log to the end of it,\n"
            "or disable session logging for this session.\n"
            "Hit Yes to wipe the file, No to append to it,\n"
            "or Cancel to disable logging.");

    switch (QMessageBox::warning(f->getMainWindow(), QString(APPNAME " Log to File"),
                         msg,
                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                         QMessageBox::Cancel)) {
    case QMessageBox::Yes:      return 2;
    case QMessageBox::No:       return 1;
    default:                    return 0;
    }
}

int get_userpass_input_v2(void *frontend, prompts_t *p, unsigned char *in, int inlen)
{
    GuiTerminalWindow *f = static_cast<GuiTerminalWindow*>(frontend);
    int ret = -1;
    //ret = cmdline_get_passwd_input(p, in, inlen);
    if (ret == -1)
    ret = term_get_userpass_input(f->term, p, in, inlen);
    return ret;
}

static void hostkey_regname(char *buffer, int buffer_sz, const char *hostname,
                int port, const char *keytype)
{
    snprintf(buffer, buffer_sz, "%s@%d:%s", keytype, port, hostname);
}

void store_host_key(const char *hostname, int port,
            const char *keytype, const char *key)
{
    char buf[200];
    hostkey_regname(buf, sizeof(buf), hostname, port, keytype);
    qutty_config.ssh_host_keys[buf] = key;
    qutty_config.saveConfig();
}

int verify_host_key(const char *hostname, int port,
            const char *keytype, const char *key)
{
    char buf[200];
    hostkey_regname(buf, sizeof(buf), hostname, port, keytype);
    if (qutty_config.ssh_host_keys.find(buf) == qutty_config.ssh_host_keys.end())
        return 1;
    if (strcmp(key, qutty_config.ssh_host_keys[buf].c_str()))
        return 2;
    return 0;
}

int verify_ssh_host_key(void *frontend, char *host, int port, char *keytype,
                        char *keystr, char *fingerprint,
                        void (*callback)(void *ctx, int result), void *ctx)
{
    assert(frontend);
    GuiTerminalWindow *f = static_cast<GuiTerminalWindow*>(frontend);
    int ret = 1;
    QString absentmsg =
        QString("The server's host key is not cached in the registry. You\n"
                "have no guarantee that the server is the computer you\n"
                "think it is.\n"
                "The server's " + QString(keytype) + " key fingerprint is:\n")
        + QString(fingerprint) + QString("\n"
                "If you trust this host, hit Yes to add the key to\n"
                APPNAME "'s cache and carry on connecting.\n"
                "If you want to carry on connecting just once, without\n"
                "adding the key to the cache, hit No.\n"
                "If you do not trust this host, hit Cancel to abandon the\n"
                "connection.\n");

    QString wrongmsg =
        QString("WARNING - POTENTIAL SECURITY BREACH!\n"
                "\n"
                "The server's host key does not match the one " APPNAME " has\n"
                "cached in the registry. This means that either the\n"
                "server administrator has changed the host key, or you\n"
                "have actually connected to another computer pretending\n"
                "to be the server.\n"
                "The new " + QString(keytype) + " key fingerprint is:\n"
                + QString(fingerprint) + "\n"
                "If you were expecting this change and trust the new key,\n"
                "hit Yes to update " APPNAME "'s cache and continue connecting.\n"
                "If you want to carry on connecting but without updating\n"
                "the cache, hit No.\n"
                "If you want to abandon the connection completely, hit\n"
                "Cancel. Hitting Cancel is the ONLY guaranteed safe\n" "choice.\n");

    /*
     * Verify the key against the registry.
     */
    ret = verify_host_key(host, port, keytype, keystr);
    if (ret == 0)		       /* success - key matched OK */
        return 1;
    else if (ret == 2) {	       /* key was different */
        switch (QMessageBox::critical(f->getMainWindow(), QString(APPNAME " Security Alert"),
                             wrongmsg,
                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                             QMessageBox::Cancel)) {
        case QMessageBox::Yes:
            store_host_key(host, port, keytype, keystr);
            return 2;
        case QMessageBox::No:       return 1;
        default:                    return 0;
        }
    } else if (ret == 1) {	       /* key was absent */
        switch (QMessageBox::warning(f->getMainWindow(), QString(APPNAME " Security Alert"),
                             absentmsg,
                             QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                             QMessageBox::Cancel)) {
        case QMessageBox::Yes:
            store_host_key(host, port, keytype, keystr);
            return 2;
        case QMessageBox::No:       return 1;
        default:                    return 0;
        }
    }
    return 0;	/* abandon the connection */
}

extern GuiMainWindow *mainWindow;

void old_keyfile_warning(void)
{
    QMessageBox::warning(mainWindow, QString(APPNAME "%s Key File Warning"),
        QString("You are loading an SSH-2 private key which has an\n"
           "old version of the file format. This means your key\n"
           "file is not fully tamperproof. Future versions of\n"
           APPNAME " may stop supporting this private key format,\n"
           "so we recommend you convert your key to the new\n"
           "format.\n"
           "\n"
           "You can perform this conversion by loading the key\n"
           "into PuTTYgen and then saving it again."),
        QMessageBox::Ok);
}

void qt_message_box_no_frontend(const char *title, const char *fmt, ...)
{
    QString msg;
    va_list args;
    va_start (args, fmt);
    QMessageBox::critical(NULL, QString(title), msg.vsprintf(fmt, args), QMessageBox::Ok);
    va_end (args);
}

void qt_message_box(void * frontend, const char *title, const char *fmt, ...)
{
    QString msg;
    va_list args;
    assert(frontend);
    GuiTerminalWindow *f = static_cast<GuiTerminalWindow*>(frontend);
    va_start (args, fmt);
    QMessageBox::critical(f, QString(title), msg.vsprintf(fmt, args), QMessageBox::Ok);
    va_end (args);
}

void logevent(void *frontend, const char *string)
{
    qDebug()<<frontend<<string;
}
