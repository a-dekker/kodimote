

/*****************************************************************************
 * Copyright: 2011-2013 Michael Zanetti <michael_zanetti@gmx.net>            *
 *            2014      Robert Meijers <robert.meijers@gmail.com>            *
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
import QtQuick 2.2
import Sailfish.Silica 1.0

Row {
    property alias title: titleText.text
    property alias text: textText.text

    width: parent.width
    spacing: Theme.paddingSmall
    visible: text.length > 0

    Label {
        id: titleText
        width: parent.width * 0.5
        horizontalAlignment: Text.AlignRight
        color: Theme.secondaryHighlightColor
        wrapMode: Text.Wrap
    }

    Label {
        width: parent.width * 0.5
        id: textText
        color: Theme.highlightColor
        // width: parent.width - titleText.width - parent.spacing
        wrapMode: Text.Wrap
    }
}
