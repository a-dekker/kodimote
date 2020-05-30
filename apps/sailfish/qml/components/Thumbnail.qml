

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

Loader {
    width: 100
    height: 100

    sourceComponent: artworkSource.length > 0 ? artworkItem : defaultTextItem

    property string artworkSource
    property string defaultText
    property string testColor: "green"
    property int fillMode: Image.Stretch
    property bool smooth: false
    property variant artworkSize: artworkSource.length > 0 ? item.sourceSize : undefined

    Component {
        id: artworkItem
        Image {
            source: artworkSource
            anchors.fill: parent
            sourceSize.height: parent.fillMode === Image.Stretch ? height : undefined
            sourceSize.width: parent.fillMode === Image.Stretch ? width : undefined
            fillMode: Image.PreserveAspectFit
            smooth: parent.smooth
        }
    }

    Component {
        id: defaultTextItem
        Text {
            text: "<b>" + defaultText + "</b> " + defaultText + " " + defaultText
            wrapMode: Text.WrapAnywhere
            anchors.fill: parent
            font.pointSize: height / 6
            clip: true
            property int colorNr: Math.floor(Math.random() * 5)
            color: colorNr == 0 ? "lightblue" : colorNr
                                  == 1 ? "red" : colorNr
                                         == 2 ? "lightgreen" : colorNr == 3 ? "yellow" : "white"
        }
    }
}
