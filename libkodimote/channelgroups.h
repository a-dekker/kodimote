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

#ifndef CHANNELGROUPS_H
#define CHANNELGROUPS_H

#include "kodilibrary.h"

class ChannelGroups : public KodiLibrary
{
    Q_OBJECT
public:
    enum ChannelType {
        ChannelTypeTV,
        ChannelTypeRadio
    };

    explicit ChannelGroups(ChannelType channelType, KodiModel *parent = 0);
    
    QString title() const;
    void refresh();
    KodiModel* enterItem(int index);
    void playItem(int index, bool resume = false);
    void addToPlaylist(int index);

    ThumbnailFormat thumbnailFormat() const { return ThumbnailFormatNone; }

signals:
    
private slots:
    void listReceived(const QVariantMap &rsp);
    
private:
    ChannelType m_channelType;
};

#endif // CHANNELGROUPSS_H
