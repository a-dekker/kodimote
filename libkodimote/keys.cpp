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

#include "keys.h"
#include "kodiconnection.h"
#include "kodi.h"
#include "eventclient.h"

Keys::Keys(QObject *parent) :
    QObject(parent)
{
    connect(KodiConnection::notifier(), SIGNAL(receivedAnnouncement(QVariantMap)), SLOT(receivedAnnouncement(QVariantMap)));
}

// Lets use the eventclient for left/right/up/down because
// Json doesn't work to skip during playback
void Keys::left()
{
    Kodi::instance()->eventClient()->sendKeypress("left");
//    KodiConnection::sendCommand("Input.Left");
}

void Keys::right()
{
    Kodi::instance()->eventClient()->sendKeypress("right");
//    KodiConnection::sendCommand("Input.Right");
}

void Keys::up()
{
    Kodi::instance()->eventClient()->sendKeypress("up");
//    KodiConnection::sendCommand("Input.Up");
}

void Keys::down()
{
    Kodi::instance()->eventClient()->sendKeypress("down");
//    KodiConnection::sendCommand("Input.Down");
}

void Keys::back()
{
    KodiConnection::sendCommand("Input.Back");
}

void Keys::osd()
{
    KodiConnection::sendCommand("Input.ShowOSD");
}

void Keys::contextMenu()
{
    KodiConnection::sendCommand("Input.ContextMenu");
}

void Keys::info()
{
    KodiConnection::sendCommand("Input.Info");
}

void Keys::fullscreen()
{
    executeAction("fullscreen");
}

void Keys::backspace()
{
    executeAction("backspace");
}

void Keys::previousMenu()
{
    executeAction("previousmenu");
}

void Keys::red()
{
    Kodi::instance()->eventClient()->sendKeypress("red");
}

void Keys::green()
{
    Kodi::instance()->eventClient()->sendKeypress("green");
}

void Keys::yellow()
{
    Kodi::instance()->eventClient()->sendKeypress("yellow");
}

void Keys::blue()
{
    Kodi::instance()->eventClient()->sendKeypress("blue");
}

void Keys::home()
{
    KodiConnection::sendCommand("Input.Home");
}

void Keys::select()
{
    KodiConnection::sendCommand("Input.Select");
}

void Keys::sendText(QString text, bool done)
{
    QVariantMap map;
    map.insert("text", text);
    map.insert("done", done);
    KodiConnection::sendCommand("Input.SendText", map);
}

void Keys::receivedAnnouncement(const QVariantMap &map)
{
    QString method = map.value("method").toString();
    QVariantMap data = map.value("params").toMap().value("data").toMap();

    if(method == "Input.OnInputRequested") {
        QString title = data.value("title").toString();
        QString type = data.value("type").toString();
        QString value = data.value("value").toString();
        emit inputRequested(title, type, value);
    }
    else if(method == "Input.OnInputFinished") {
        emit inputFinished();
    }
}

QString Keys::formatTime(int hours, int minutes)
{
    return QString("%1:%2").arg(hours, 2, 10, QChar('0')).arg(minutes, 2, 10, QChar('0'));

}
void Keys::executeAction(const QString &action)
{
    QVariantMap params;
    params.insert("action", action);
    KodiConnection::sendCommand("Input.ExecuteAction", params);
}
