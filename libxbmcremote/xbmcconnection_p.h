/*****************************************************************************
 * Copyright: 2011 Michael Zanetti <mzanetti@kde.org>                        *
 *                                                                           *
 * This program is free software: you can redistribute it and/or modify      *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * This program is distributed in the hope that it will be useful,           *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                           *
 ****************************************************************************/

#ifndef XBMC_P_H
#define XBMC_P_H

#include "xbmcconnection.h"
#include "xbmchostmodel.h"
//#include "player.h"

#include <QString>
#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QDate>
#include <QFile>
#include <QPointer>
#include <QNetworkConfigurationManager>
#include <QNetworkSession>

class XbmcDownload;

namespace XbmcConnection
{

class Command
{
public:
    Command(int id = -1, const QString &command = QString(), const QVariant &params = QVariant(), const QString &raw = QString()):
        m_id(id), m_command(command), m_params(params), m_raw(raw) {}

    int id() {return m_id;}
    QString command() {return m_command;}
    QVariant params() {return m_params;}
    QString raw() {return m_raw; }

private:

    int m_id;
    QString m_command;
    QVariant m_params;
    QString m_raw;
};

class Callback
{
public:
    Callback() {}
    Callback(QPointer<QObject> receiver, const QString &member):
        m_receiver(receiver), m_member(member) {}

    QPointer<QObject> receiver() { return m_receiver; }
    QString member() { return m_member; }

private:
    QPointer<QObject> m_receiver;
    QString m_member;
};

class XbmcConnectionPrivate : public QObject
{
    Q_OBJECT
public:
    explicit XbmcConnectionPrivate(QObject *parent = 0);

    XbmcHost *connectedHost();
    bool connecting();
    bool connected();
    QString connectionError();
    void setAuthCredentials(const QString &username, const QString &password);

    int sendCommand(const QString &command, const QVariant &parms = QVariant());
    int sendCommand(const QString &command, const QVariant &params, QObject *callbackReceiver, const QString &callbackMember);
    void sendLegacyCommand(const QString &command);

    QNetworkAccessManager *nam();
    Notifier *notifier();

    void download(XbmcDownload *download);

public slots:
    void connect(XbmcHost *host = 0);

private slots:
    void readData();
    void clearPending();
    void socketError();
    void slotConnected();
    void slotDisconnected();
    void internalConnect();
    void sessionLost();

    void replyReceived();
    void authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);

    void downloadNext();
    void downloadReadyRead();
    void downloadFinished();
    void downloadProgress(qint64 progress, qint64 total);

    void cancelDownload();

private:
    QTcpSocket *m_socket;
    int m_commandId;
    Notifier *m_notifier;
    int m_versionRequestId;
    int m_xbmcVersionMajor;
    int m_xbmcVersionMinor;

    QList<Command> m_commandQueue;
    Command m_currentPendingCommand;
    QTimer m_timeoutTimer;
    QTimer m_reconnectTimer;

    void sendNextCommand();
    void sendNextCommand2();

    XbmcHost *m_host;

    QNetworkAccessManager *m_network;
    QNetworkReply *m_lastAuthRequest;
    bool m_connecting;
    bool m_connected;
    QString m_connectionError;
    QMap<int, Callback> m_callbacks;
    QNetworkConfigurationManager *m_connManager;
    QNetworkSession *m_networkSession;

    QList<XbmcDownload*> m_downloadQueue;
    QMap<QNetworkReply*, XbmcDownload*> m_activeDownloadsMap;

};
Q_GLOBAL_STATIC(XbmcConnectionPrivate, instance)


}
#endif // XBMC_P_H