/*****************************************************************************
 * Copyright: 2011-2013 Michael Zanetti <michael_zanetti@gmx.net>            *
 *                                                                           *
 * This file is part of Kodimote                                           *
 *                                                                           *
 * Kodimote is free software: you can redistribute it and/or modify        *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Kodimote is distributed in the hope that it will be useful,             *
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

#include "kodiconnection.h"
#include "kodihostmodel.h"
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
#include <QNetworkReply>

class KodiDownload;

namespace KodiConnection
{

class Command
{
public:
    Command(int id = -1, const QString &command = QString(), const QVariant &params = QVariant(), const QString &raw = QString()):
        m_id(id), m_command(command), m_params(params), m_raw(raw) {}

    int id() const {return m_id;}
    QString command() const {return m_command;}
    QVariant params() const {return m_params;}
    QString raw() const {return m_raw; }

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

class KodiConnectionPrivate : public QObject
{
    Q_OBJECT
public:
    explicit KodiConnectionPrivate(QObject *parent = 0);
    static KodiConnectionPrivate *instance();

    KodiHost *connectedHost();
    bool connecting();
    bool connected();
    QString connectionError();
    void disconnectFromHost();
    void setAuthCredentials(const QString &username, const QString &password);

    bool active() const;
    void setActive(bool active);

    int sendCommand(const QString &command, const QVariant &parms = QVariant());
    int sendCommand(const QString &command, const QVariant &params, QObject *callbackReceiver, const QString &callbackMember);
    int sendParallelCommand(const QString &command, const QVariant &params, QObject *callbackReceiver, const QString &callbackMember);

    QNetworkAccessManager *nam();
    Notifier *notifier();

    void download(KodiDownload *download);

public slots:
    void connect(KodiHost *host = 0);

private slots:
    void readData();
    void clearPending();
    void socketError();
    void requestError(QNetworkReply::NetworkError error);
    void slotConnected();
    void slotDisconnected();
    void internalConnect();
    void sessionLost();
    void versionReceived(const QVariantMap &rsp);

    void pingElapsed();
    void pingReplyReceived(const QVariantMap &rsp);

    void replyReceived();
    void authenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);

    void downloadNext();
    void downloadReadyRead();
    void downloadFinished();
    void downloadProgress(qint64 progress, qint64 total);

    void cancelDownload();

private:
    QTcpSocket *m_socket;
    QString m_scopeId;
    int m_commandId;
    Notifier *m_notifier;
    int m_versionRequestId;
    int m_kodiVersionMajor;
    int m_kodiVersionMinor;

    QList<Command> m_commandQueue;
    Command m_currentPendingCommand;
    QTimer m_timeoutTimer;
    QTimer m_reconnectTimer;
    QTimer m_pingTimeoutTimer;

    void sendNextCommand();
    void handleData(const QString &data);
    void closeConnection(bool reconnect = true);
    QByteArray buildJsonPayload(const Command &command);
    QByteArray sendRequest(const Command &command);

    KodiHost *m_host;

    QNetworkAccessManager *m_network;
    QNetworkReply *m_lastAuthRequest;
    bool m_connecting;
    bool m_connected;
    bool m_disconnecting;
    QString m_connectionError;
    QMap<int, Callback> m_callbacks;
    QNetworkConfigurationManager *m_connManager;
    QNetworkSession *m_networkSession;
    bool m_active;

    QList<KodiDownload*> m_downloadQueue;
    QMap<QNetworkReply*, KodiDownload*> m_activeDownloadsMap;
};

}
#endif // XBMC_P_H
