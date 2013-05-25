/*
 * Copyright (C) 2012 Rajendran Thirupugalsamy
 * See LICENSE for full copyright and license information.
 * See COPYING for distribution information.
 */

#include "QtCommon.h"
#include "GuiTerminalWindow.h"
extern "C" {
#include "ssh.h"
#include "putty.h"
#include "network.h"
}
#include <QHostAddress>
#include <QHostInfo>
#include <QNetworkInterface>

struct SockAddr_tag {
    QHostAddress *qtaddr;
    const char *error;
};

static void sk_tcp_flush(Socket s) {}

/*
 * Each socket abstraction contains a `void *' private field in
 * which the client can keep state.
 */
static void sk_tcp_set_private_ptr(Socket sock, void *ptr)
{
    Actual_Socket s = (Actual_Socket) sock;
    s->private_ptr = ptr;
}

static void *sk_tcp_get_private_ptr(Socket sock)
{
    Actual_Socket s = (Actual_Socket) sock;
    return s->private_ptr;
}

static const char *sk_tcp_socket_error(Socket sock)
{
    Actual_Socket s = (Actual_Socket) sock;
    return s->error;
}

static int sk_tcp_write (Socket sock, const char *data, int len)
{
    Actual_Socket s = (Actual_Socket) sock;
    int i,j;
    char pr[10000];
    for(i=0, j=0; i<len; i++)
            j+= sprintf(pr+j, "%u ", (unsigned char)data[i]);
    //qDebug()<<"sk_tcp_write"<<len<<pr;
    int ret = s->qtsock->write(data, len);
    noise_ultralight(len);
    if(ret<=0)
        qDebug()<<"tcp_write ret "<<ret;
    return ret;
}

static int sk_tcp_write_oob (Socket sock, const char *data, int len)
{
    Actual_Socket s = (Actual_Socket) sock;
    int ret = s->qtsock->write(data, len);
    qDebug()<<"tcp_write_oob ret "<<ret<<"\n";
    return ret;
}

static void sk_tcp_close (Socket sock)
{
    Actual_Socket s = (Actual_Socket) sock;
    s->qtsock->disconnectFromHost();
}

static void sk_tcp_set_frozen (Socket sock, int is_frozen)
{
    Actual_Socket s = (Actual_Socket) sock;
    qDebug()<<"sk_tcp_set_frozen() NOT IMPL\n";
}

static Plug sk_tcp_plug (Socket sock, Plug p)
{
    Actual_Socket s = (Actual_Socket) sock;
    Plug ret = s->plug;
    if (p)
        s->plug = p;
    return ret;
}


Socket sk_new(char *addr, int port, int privport, int oobinline,
              int nodelay, int keepalive, Plug plug)
{
    static const struct socket_function_table fn_table = {
        sk_tcp_plug,
        sk_tcp_close,
        sk_tcp_write,
        sk_tcp_write_oob,
        sk_tcp_flush,
        sk_tcp_set_private_ptr,
        sk_tcp_get_private_ptr,
        sk_tcp_set_frozen,
        sk_tcp_socket_error
    };

    Actual_Socket ret;

    /*
     * Create Socket structure.
     */
    ret = snew(struct Socket_tag);
    ret->fn = &fn_table;
    ret->error = NULL;
    ret->plug = plug;
    bufchain_init(&ret->output_data);
    ret->connected = 0;		       /* to start with */
    ret->writable = 0;		       /* to start with */
    ret->sending_oob = 0;
    ret->frozen = 0;
    ret->frozen_readable = 0;
    ret->localhost_only = 0;	       /* unused, but best init anyway */
    ret->pending_error = 0;
    ret->parent = ret->child = NULL;
    ret->oobinline = oobinline;
    ret->nodelay = nodelay;
    ret->keepalive = keepalive;
    ret->privport = privport;
    ret->port = port;
    //ret->addr = addr;
    //START_STEP(ret->addr, ret->step);
    //ret->s = INVALID_SOCKET;

    /*do {
        err = try_connect(ret);
    } while (err && sk_nextaddr(ret->addr, &ret->step));
    */
    ret->qtsock = new QTcpSocket();
    ret->qtsock->connectToHost(QString(addr), port);

    if(nodelay)
        ret->qtsock->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    return (Socket) ret;
}

int sk_addrtype(SockAddr addr)
{
    const QHostAddress *a = addr->qtaddr;
    switch(a->protocol()) {
    case QAbstractSocket::IPv4Protocol:
        return ADDRTYPE_IPV4;
    case QAbstractSocket::IPv6Protocol:
        return ADDRTYPE_IPV6;
    default:
        return ADDRTYPE_NAME;
    }
}

void sk_addrcopy(SockAddr addr, char *buf)
{
    QHostAddress *a = addr->qtaddr;
    QString str = a->toString();
    QByteArray bstr = str.toUtf8();
    const char* cstr = bstr.constData();
    memcpy(buf, cstr, bstr.length());
}

SockAddr sk_addr_dup(SockAddr addr)
{
    if(!addr) return NULL;
    SockAddr ret = new SockAddr_tag;
    ret->qtaddr = new QHostAddress(*addr->qtaddr);
    ret->error = addr->error;
    return ret;
}

void sk_addr_free(SockAddr addr)
{
    if (!addr)
        return;
    if (addr->qtaddr)
        delete addr->qtaddr;
    addr->qtaddr = NULL;
    addr->error = NULL;
}

SockAddr sk_namelookup(const char *host, char **canonicalname,
               int address_family)
{
     SockAddr ret = new SockAddr_tag;
     ret->error = NULL;
     QHostInfo info = QHostInfo::fromName(host);
     if (info.error() == QHostInfo::NoError) {
         foreach(const QHostAddress &address, info.addresses()) {
             int this_addrtype = address.protocol() == QAbstractSocket::IPv4Protocol ? ADDRTYPE_IPV4 :
                                 address.protocol() == QAbstractSocket::IPv6Protocol ? ADDRTYPE_IPV6 :
                                 ADDRTYPE_UNSPEC;
             if (this_addrtype == address_family) {
                 QHostAddress *a = new QHostAddress(address);
                 QString str = info.hostName();
                 QByteArray bstr = str.toUtf8();
                 const char* cstr = bstr.constData();
                 *canonicalname = snewn(1+strlen(cstr), char);
                 strcpy(*canonicalname, cstr);
                 ret->qtaddr = a;
                 return ret;
             }
         }
     }
     if (info.addresses().size()>0) {
         QHostAddress *a = new QHostAddress(info.addresses().at(0));
         QString str = info.hostName();
         QByteArray bstr = str.toUtf8();
         const char* cstr = bstr.constData();
         *canonicalname = snewn(1+strlen(cstr), char);
         strcpy(*canonicalname, cstr);
         ret->qtaddr = a;
         return ret;
     }
     *canonicalname = snewn(1, char);
     *canonicalname[0] = '\0';
     ret->qtaddr = new QHostAddress();
     ret->error = info.error()==QHostInfo::HostNotFound ? "Host not found" :
                  info.error()==QHostInfo::UnknownError ? "Unknown error" :
                  "No IP address found";
     return ret;
}

SockAddr sk_nonamelookup(const char *host)
{
    // TODO not supported for now
    SockAddr ret = new SockAddr_tag;
    ret->qtaddr = new QHostAddress();
    ret->error = "Not supported";
    return ret;
}

const char *sk_addr_error(SockAddr addr)
{
    if (!addr) return NULL;
    return addr->error;
}

int sk_hostname_is_local(char *name)
{
    return !strcmp(name, "localhost") ||
       !strcmp(name, "::1") ||
       !strncmp(name, "127.", 4);
}

void sk_getaddr(SockAddr addr, char *buf, int buflen)
{
    QHostAddress *a = addr->qtaddr;
    QString str = a->toString();
    QByteArray bstr = str.toUtf8();
    const char* cstr = bstr.constData();
    if(buflen>bstr.length()) buflen = bstr.length();
    strncpy(buf, cstr, buflen);
    buf[buflen-1] = '\0';
}

int sk_address_is_local(SockAddr addr)
{
    const QHostAddress *a = addr->qtaddr;
    if (*a==QHostAddress::LocalHost || *a==QHostAddress::LocalHostIPv6)
        return 1;
    foreach(const QHostAddress &locaddr, QNetworkInterface::allAddresses()) {
        if (*a==locaddr)
            return 1;
    }
    return 0;
}

Socket sk_new(SockAddr addr, int port, int privport, int oobinline,
          int nodelay, int keepalive, Plug plug)
{
    static const struct socket_function_table fn_table = {
        sk_tcp_plug,
        sk_tcp_close,
        sk_tcp_write,
        sk_tcp_write_oob,
        sk_tcp_flush,
        sk_tcp_set_private_ptr,
        sk_tcp_get_private_ptr,
        sk_tcp_set_frozen,
        sk_tcp_socket_error
    };

    Actual_Socket ret;

    /*
     * Create Socket structure.
     */
    ret = snew(struct Socket_tag);
    ret->fn = &fn_table;
    ret->error = NULL;
    ret->plug = plug;
    bufchain_init(&ret->output_data);
    ret->connected = 0;		       /* to start with */
    ret->writable = 0;		       /* to start with */
    ret->sending_oob = 0;
    ret->frozen = 0;
    ret->frozen_readable = 0;
    ret->localhost_only = 0;	       /* unused, but best init anyway */
    ret->pending_error = 0;
    ret->parent = ret->child = NULL;
    ret->oobinline = oobinline;
    ret->nodelay = nodelay;
    ret->keepalive = keepalive;
    ret->privport = privport;
    ret->port = port;
    ret->addr = addr;
    ret->qtsock = NULL;

    if (!addr || !addr->qtaddr) {
        ret->error = "Cannot create socket";
        goto cu0;
    }

    ret->qtsock = new QTcpSocket();
    ret->qtsock->connectToHost(*addr->qtaddr, port);

    if(nodelay)
        ret->qtsock->setSocketOption(QAbstractSocket::LowDelayOption, 1);

cu0:
    return (Socket) ret;
}

Socket sk_newlistener(char *srcaddr, int port, Plug plug, int local_host_only,
              int orig_address_family)
{
    // TODO not implemented
    return NULL;
}

char *get_hostname(void)
{
    static char hostname[512];
    QString str = QHostInfo::localHostName();
    QByteArray bstr = str.toUtf8();
    const char* cstr = bstr.constData();
    memcpy(hostname, cstr, bstr.length());
    return hostname;
}

int net_service_lookup(char *service)
{
    // TODO not implemented
    return 0;
}

Socket sk_register(void *sock, Plug plug)
{
    // TODO not implemented
    return NULL;
}

SockAddr platform_get_x11_unix_address(const char *path, int displaynum)
{
    /*
    SockAddr ret = snew(struct SockAddr_tag);
    memset(ret, 0, sizeof(struct SockAddr_tag));
    ret->error = "unix sockets not supported on this platform";
    ret->refcount = 1;
    return ret;
    */
    return NULL;
}

Socket platform_new_connection(SockAddr addr, char *hostname,
                   int port, int privport,
                   int oobinline, int nodelay, int keepalive,
                   Plug plug, const Config *cfg)
{
    // TODO not yet implemented
    return NULL;
}
