/*****************************************************************************
 * Copyright: 2011-2013 Michael Zanetti <michael_zanetti@gmx.net>            *
 *                                                                           *
 * This file is part of Xbmcremote                                           *
 *                                                                           *
 * Xbmcremote is free software: you can redistribute it and/or modify        *
 * it under the terms of the GNU General Public License as published by      *
 * the Free Software Foundation, either version 3 of the License, or         *
 * (at your option) any later version.                                       *
 *                                                                           *
 * Xbmcremote is distributed in the hope that it will be useful,             *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
 * GNU General Public License for more details.                              *
 *                                                                           *
 * You should have received a copy of the GNU General Public License         *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
 *                                                                           *
 ****************************************************************************/

#include "libxbmcremote/xbmchostmodel.h"
#include "libxbmcremote/xbmc.h"
#include "libxbmcremote/settings.h"

#include "ubuntuhelper.h"

UbuntuHelper::UbuntuHelper(Settings *settings, QObject *parent) :
    QObject(parent),
    m_settings(settings)
{
    // Load stored hosts
    foreach(const XbmcHost &host, settings->hostList()) {
        int index = Xbmc::instance()->hostModel()->insertOrUpdateHost(host);
        if(host.address() == settings->lastHost().address()) {
            Xbmc::instance()->hostModel()->connectToHost(index);
            qDebug() << "reconnecting to" << host.hostname() << host.address() << host.username() << host.password();
        }
    }

    connect(Xbmc::instance(), SIGNAL(connectedChanged(bool)), SLOT(connectionChanged(bool)));
    connect(Xbmc::instance()->hostModel(), SIGNAL(rowsRemoved(QModelIndex, int, int)), SLOT(hostRemoved()));
}

void UbuntuHelper::connectionChanged(bool connected)
{
    if(connected) {
        m_settings->addHost(*Xbmc::instance()->connectedHost());
        m_settings->setLastHost(*Xbmc::instance()->connectedHost());
    }

}

void UbuntuHelper::hostRemoved()
{
    // We need to check if all our stored hosts are still in hostList
    for(int i = 0; i < m_settings->hostList().count();) {
        bool found = false;
        for(int j = 0; j < Xbmc::instance()->hostModel()->rowCount(QModelIndex()); ++j) {
            if(m_settings->hostList().at(i).address() == Xbmc::instance()->hostModel()->get(j, "address").toString()) {
                found = true;
                break;
            }
        }
        if(!found) {
            m_settings->removeHost(m_settings->hostList().at(i));
            qDebug() << "removed host" << i;
        } else {
            ++i;
        }
    }
}
