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

#include "player.h"
#include "playlist.h"
#include "libraryitem.h"

#include "xbmcconnection.h"
#include "xbmc.h"

#include "xdebug.h"

#include <QTime>

Player::Player(PlayerType type, QObject *parent) :
    QObject(parent),
    m_type(type),
    m_state("stopped"),
    m_speed(1),
    m_percentage(0),
    m_lastPlaytime(0),
    m_lastPlaytimeUpdate(QDateTime::currentDateTime()),
    m_currentItem(new LibraryItem()),
    m_seeking(false),
    m_shuffle(false),
    m_repeat(RepeatNone)
{
    connect(XbmcConnection::notifier(), SIGNAL(receivedAnnouncement(QVariantMap)), SLOT(receivedAnnouncement(QVariantMap)));

    m_playtimeTimer.setInterval(1000);
    connect(&m_playtimeTimer, SIGNAL(timeout()), SLOT(updatePlaytime()));
}

void Player::getSpeed()
{
    QVariantMap params;
    params.insert("playerid", playerId());
    QVariantList properties;
    properties.append("speed");
    params.insert("properties", properties);
    XbmcConnection::sendCommand("Player.GetProperties", params, this, "speedReceived");
}

void Player::getPlaytime()
{
    QVariantMap params;
    params.insert("playerid", playerId());
    QVariantList props;
    props.append("time");
    params.insert("properties", props);
    XbmcConnection::sendCommand("Player.GetProperties", params, this, "playtimeReceived");
}

void Player::getPosition()
{
    QVariantMap params;
    params.insert("playerid", playerId());
    QVariantList props;
    props.append("position");
    params.insert("properties", props);
    XbmcConnection::sendCommand("Player.GetProperties", params, this, "positionReceived");
}

void Player::getRepeatShuffle()
{
    QVariantMap params;
    params.insert("playerid", playerId());
    QVariantList props;
    props.append("repeat");
    props.append("shuffled");
    params.insert("properties", props);
    XbmcConnection::sendCommand("Player.GetProperties", params, this, "repeatShuffleReceived");
}

void Player::getCurrentItemDetails()
{
    QVariantMap params;
    params.insert("playerid", playerId());

    QVariantList properties;
    properties.append("title");
    properties.append("artist");
    properties.append("comment");
    properties.append("genre");
    properties.append("season");
    properties.append("rating");
    properties.append("episode");
    properties.append("year");
    properties.append("director");
    properties.append("tagline");
    properties.append("mpaa");
    properties.append("album");
    properties.append("fanart");
    properties.append("file");

    properties.append("showtitle");
//    properties.append("style");
//    properties.append("mood");
//    properties.append("born");
//    properties.append("formed");
//    properties.append("died");
//    properties.append("disbanded");
    properties.append("duration");
    properties.append("playcount");
    properties.append("cast");
    properties.append("plot");
//    properties.append("description");
    properties.append("thumbnail");
    properties.append("runtime");
    params.insert("properties", properties);

    XbmcConnection::sendCommand("Player.GetItem", params, this, "detailsReceived");
}

void Player::refresh()
{
    xDebug(XDAREA_PLAYER) << "player" << playerId() << "refreshing";
    QVariantMap params;
    params.insert("playerid", playerId());
    QVariantList props;
    props.append("speed");
    props.append("time");
    props.append("position");
    props.append("repeat");
    props.append("shuffled");
    params.insert("properties", props);
    XbmcConnection::sendCommand("Player.GetProperties", params, this, "refreshReceived");
    getCurrentItemDetails();
    playlist()->refresh();
}

void Player::detach()
{
    xDebug(XDAREA_PLAYER) << "stopping player";
    m_state = "stopped";
    m_playtimeTimer.stop();
    emit stateChanged();
    m_speed = 1;
    emit speedChanged();
    playlist()->refresh();
}

Player::PlayerType Player::type() const
{
    return m_type;
}


void Player::playPause()
{
    QVariantMap params;
    params.insert("playerid", playerId());

    XbmcConnection::sendCommand("Player.PlayPause", params);
}

void Player::stop()
{
    QVariantMap params;
    params.insert("playerid", playerId());
    XbmcConnection::sendCommand("Player.Stop", params);
}

void Player::skipPrevious()
{
    QVariantMap params;
    params.insert("playerid", playerId());
    params.insert("to", "previous");

    XbmcConnection::sendCommand("Player.GoTo", params);
}

void Player::skipNext()
{
    QVariantMap params;
    params.insert("playerid", playerId());
    params.insert("to", "next");

    XbmcConnection::sendCommand("Player.GoTo", params);
}

void Player::seekBackward()
{
    QVariantMap params;
    params.insert("playerid", playerId());
    params.insert("speed", "decrement");
    XbmcConnection::sendCommand("Player.SetSpeed", params);
}

void Player::seekForward()
{
    QVariantMap params;
    params.insert("playerid", playerId());
    params.insert("speed", "increment");
    XbmcConnection::sendCommand("Player.SetSpeed", params);
}

QString Player::state() const
{
    return m_state;
}

void Player::receivedAnnouncement(const QVariantMap &map)
{
    QVariantMap data= map.value("params").toMap().value("data").toMap();
    if(!data.value("player").isValid() || data.value("player").toMap().value("playerid").toInt() != playerId()) {
        return;
    }

    xDebug(XDAREA_PLAYER) << "Player" << playerId() << "got announcement:" << map.value("method") << data;

    if(map.value("method").toString() == "Player.OnStop") {
        //is most likely unreachable as Player.OnStop doesn't contain a player id
        // and thus it would already return above
        detach();
    } else if(map.value("method").toString() == "Player.OnPause") {
        m_state = "paused";
        m_playtimeTimer.stop();
        updatePlaytime();
        emit stateChanged();
        m_speed = 1;
        emit speedChanged();
    } else if(map.value("method").toString() == "Player.OnPlay") {
        m_state = "playing";
        emit stateChanged();
        m_speed = data.value("player").toMap().value("speed").toInt();
        qDebug() << "set speed to" << m_speed;
        emit speedChanged();
        getPosition();
        getPlaytime();
        getCurrentItemDetails();
        m_lastPlaytimeUpdate = QDateTime::currentDateTime();
        if(m_timerActivated) {
            m_playtimeTimer.start();
        }
        playlist()->refresh();
    } else if(map.value("method").toString() == "Player.OnSeek") {
        updatePlaytime(data.value("player").toMap().value("time").toMap());
        m_seeking = false;
    } else if(map.value("method").toString() == "Player.OnSpeedChanged") {
        updatePlaytime();
        m_speed = data.value("player").toMap().value("speed").toInt();
        emit speedChanged();
    }
}

void Player::speedReceived(const QVariantMap &rsp)
{
    m_speed = rsp.value("result").toMap().value("speed").toInt();
    xDebug(XDAREA_PLAYER) << "got player speed" << m_speed;
    emit speedChanged();

    if(m_speed == 0) {
        m_state = "paused";
    } else {
        m_state = "playing";
    }
    emit stateChanged();
}

void Player::playtimeReceived(const QVariantMap &rsp)
{
    xDebug(XDAREA_PLAYER) << "Got playtime response" << rsp;
    updatePlaytime(rsp.value("result").toMap().value("time").toMap());
}

void Player::positionReceived(const QVariantMap &rsp)
{
    xDebug(XDAREA_PLAYER) << "Got position response" << rsp;
    playlist()->setCurrentIndex(rsp.value("result").toMap().value("position").toInt());
}

void Player::repeatShuffleReceived(const QVariantMap &rsp)
{
    QVariant result = rsp.value("result");
    if(result.toMap().value("repeat").toString() == "off") {
        m_repeat = RepeatNone;
    } else if(result.toMap().value("repeat").toString() == "one") {
        m_repeat = RepeatOne;
    } else {
        m_repeat = RepeatAll;
    }
    emit repeatChanged();

    m_shuffle = result.toMap().value("shuffled").toBool();
    emit shuffleChanged();
}

void Player::refreshReceived(const QVariantMap &rsp)
{
    speedReceived(rsp);
    playtimeReceived(rsp);
    positionReceived(rsp);
    repeatShuffleReceived(rsp);
}

void Player::detailsReceived(const QVariantMap &rsp)
{
    xDebug(XDAREA_PLAYER) << "got current item details:" << rsp;
    QVariantMap itemMap = rsp.value("result").toMap().value("item").toMap();

    if(m_currentItem) {
        m_currentItem->deleteLater();
    }
    m_currentItem = new LibraryItem();

    int id = itemMap.value("id").toInt();
    QString type = itemMap.value("type").toString();
    if(type == "episode") {
        m_currentItem->setEpisodeId(id);
    } else if(type == "movie") {
        m_currentItem->setMovieId(id);
    } else if(type == "musicvideo") {
        m_currentItem->setMusicvideoId(id);
    } else if(type == "song") {
        m_currentItem->setSongId(id);
    } else if(type == "album") {
        m_currentItem->setAlbumId(id);
    } else if(type == "artist") {
        m_currentItem->setArtistId(id);
    } else if(type == "channel") {
        qDebug() << "mmmmmmmmmmmmmmmmmmmmm" << id;
        m_currentItem->setChannelId(id);
    }

    m_currentItem->setType(type);


    m_currentItem->setTitle(itemMap.value("label").toString());
    if(itemMap.value("type").toString() == "song") {
        m_currentItem->setSubtitle(itemMap.value("artist").toString());
    } else if(itemMap.value("type").toString() == "episode") {
        m_currentItem->setSubtitle(itemMap.value("showtitle").toString());
    } else if(itemMap.value("type").toString() == "channel") {
        m_currentItem->setTitle(itemMap.value("title").toString());
        m_currentItem->setSubtitle(itemMap.value("label").toString());
    }

    m_currentItem->setComment(itemMap.value("comment").toString());
    m_currentItem->setGenre(itemMap.value("genre").toString());
    m_currentItem->setSeason(itemMap.value("season", -1).toInt());
    m_currentItem->setRating(itemMap.value("rating", -1).toInt());
    m_currentItem->setEpisode(itemMap.value("episode", -1).toInt());
    m_currentItem->setYear(itemMap.value("year").toString());
    m_currentItem->setDirector(itemMap.value("director").toString());
    m_currentItem->setTagline(itemMap.value("tagline").toString());
    m_currentItem->setMpaa(itemMap.value("mpaa").toString());
    m_currentItem->setInstrument(itemMap.value("instrument").toString());
    m_currentItem->setStyle(itemMap.value("style").toString());
    m_currentItem->setMood(itemMap.value("mood").toString());
    m_currentItem->setBorn(itemMap.value("born").toString());
    m_currentItem->setFormed(itemMap.value("formed").toString());
    m_currentItem->setDied(itemMap.value("died").toString());
    m_currentItem->setDisbanded(itemMap.value("disbanded").toString());
    if(itemMap.contains("runtime")) {
        m_currentItem->setDuration(QTime(0, 0, 0).addSecs(itemMap.value("runtime").toDouble()));
        qDebug() << "set runtime 1" << m_currentItem->duration() << itemMap.value("runtime");
    } else  {
        m_currentItem->setDuration(QTime(0, 0, 0).addSecs(itemMap.value("duration").toInt()));
        qDebug() << "set runtime 2" << m_currentItem->duration();
    }
    m_currentItem->setPlaycount(itemMap.value("playcount", -1).toInt());
    m_currentItem->setCast(itemMap.value("cast").toString());
    m_currentItem->setPlot(itemMap.value("plot").toString());
    m_currentItem->setThumbnail(itemMap.value("thumbnail").toString());
    m_currentItem->setAlbum(itemMap.value("album").toString());
    m_currentItem->setArtist(itemMap.value("artist").toString());
    m_currentItem->setFanart(itemMap.value("fanart").toString());
    m_currentItem->setTvShow(itemMap.value("showtitle").toString());
    m_currentItem->setFileName(itemMap.value("file").toString());
    emit currentItemChanged();
}

int Player::speed() const
{
    return m_speed;
}

double Player::percentage() const
{
    return m_percentage;
}

QString Player::time() const
{
    if(m_currentItem) {
        QTime time = QTime(0, 0, 0).addMSecs(m_lastPlaytime);
        if(m_currentItem->duration().hour() > 0) {
            return time.toString("hh:mm:ss");
        } else {
            return time.toString("mm:ss");
        }
    }
    return "00:00";
}

void Player::updatePlaytime()
{
    if(!m_currentItem) {
        return;
    }

    //use milliseconds, otherwise it tends to skip a sec. once in a while
    int duration = QTime(0, 0, 0).msecsTo(m_currentItem->duration());
    QDateTime currentTime = QDateTime::currentDateTime();
    int elapsedMSeconds = m_lastPlaytimeUpdate.msecsTo(currentTime);
    m_lastPlaytime += elapsedMSeconds * m_speed;
    m_percentage = (double)m_lastPlaytime / duration * 100;
    m_lastPlaytimeUpdate = currentTime;

    emit percentageChanged();
    emit timeChanged();
}

void Player::updatePlaytime(const QVariantMap &timeMap)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QTime time;
    int hours = timeMap.value("hours").toInt();
    int minutes = timeMap.value("minutes").toInt();
    int seconds = timeMap.value("seconds").toInt();
    int mseconds = timeMap.value("milliseconds").toInt();
    time.setHMS(hours, minutes, seconds, mseconds);
    m_lastPlaytime = QTime(0, 0, 0).msecsTo(time);
    m_lastPlaytimeUpdate = currentTime;
    if(playlist()->currentItem()) {
        int duration = QTime(0, 0, 0).msecsTo(playlist()->currentItem()->duration());
        m_percentage = (double)m_lastPlaytime / duration * 100;
        emit percentageChanged();
    }

    emit timeChanged();
}

void Player::playItem(int index)
{
    QVariantMap params;
    QVariantMap playlistid;
    playlistid.insert("playlistid", playlist()->playlistId());
    playlistid.insert("position", index);
    params.insert("item", playlistid);
    XbmcConnection::sendCommand("Player.Open", params);
}

bool Player::shuffle() const
{
    return m_shuffle;
}

void Player::setShuffle(bool shuffle)
{
    QVariantMap params;
    params.insert("playerid", playerId());
    params.insert("shuffle", shuffle);

    XbmcConnection::sendCommand("Player.SetShuffle", params);
    getRepeatShuffle();
}

void Player::setRepeat(Player::Repeat repeat)
{
    QVariantMap params;
    switch(repeat) {
    case RepeatNone:
        params.insert("repeat", "off");
        break;
    case RepeatOne:
        params.insert("repeat", "one");
        break;
    case RepeatAll:
        params.insert("repeat", "all");
        break;
    }
    params.insert("playerid", playerId());

    XbmcConnection::sendCommand("Player.SetRepeat", params);

    getRepeatShuffle();
}

Player::Repeat Player::repeat() const
{
    return m_repeat;
}

bool Player::timerActive() const
{
    return m_playtimeTimer.isActive();
}

void Player::setTimerActive(bool active)
{
    if(m_playtimeTimer.isActive() == active) {
        return;
    }

    m_timerActivated = active;

    if(active) {
        if(m_state == "playing") {
            m_playtimeTimer.start();
            updatePlaytime();
        }
    }
    else {
        m_playtimeTimer.stop();
    }
}

void Player::seek(int position)
{
    if(m_seeking && position != m_percentage) {
        return;
    }
    QVariantMap params;
    params.insert("playerid", playerId());
    params.insert("value", position);

    m_seeking = true;
    XbmcConnection::sendCommand("Player.Seek", params);
}

LibraryItem *Player::currentItem() const
{
    return m_currentItem;
}