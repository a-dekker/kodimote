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

#ifndef KEYS_H
#define KEYS_H

#include <QObject>
#include <QVariantMap>

class Keys : public QObject
{
    Q_OBJECT
public:
    explicit Keys(QObject *parent = 0);

public slots:
    void left();
    void right();
    void up();
    void down();
    void home();
    void select();
    void back();
    void osd();
    void contextMenu();
    void info();
    void fullscreen();
    void backspace();
    void sendText(QString text, bool done = true);

    QString formatTime(int hours, int minutes);

signals:
    void inputFinished();
    void inputRequested(QString title, QString type, QString value);

private slots:
    void receivedAnnouncement(const QVariantMap& map);

    void red();
    void green();
    void yellow();
    void blue();

private:
    void executeAction(const QString &action);
    int m_requestId;
};

#endif // KEYS_H
