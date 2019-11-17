

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

Item {
    id: noConnection

    signal showConnect

    width: (parent ? parent.width : screen.width) - 2 * Theme.paddingLarge
    height: _content ? _content.height : 0
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter
    visible: false

    onVisibleChanged: {
        if (visible && !_content) {
            _content = activeContent.createObject(noConnection)
        }
    }

    property Item _content

    Component {
        id: activeContent

        Column {
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width
            spacing: 20

            Label {
                id: label
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width - 100
                text: kodi.connectionError
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                color: Theme.highlightColor
            }

            Label {
                visible: !!kodi.connectionError
                width: parent.width - (Theme.paddingLarge * 2)
                anchors.horizontalCenter: parent.horizontalCenter
                wrapMode: Text.WordWrap
                text: qsTr("Please enable the following options in the Services settings of Kodi:")
                      + "\n- " + qsTr(
                          "Allow control of Kodi via HTTP") + "\n- " + qsTr(
                          "Allow programs on other systems to control Kodi")
                color: Theme.highlightColor
            }

            Button {
                text: qsTr("Connect")
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: noConnection.showConnect()
            }

            Button {
                text: qsTr("Wake host")
                anchors.horizontalCenter: parent.horizontalCenter
                visible: kodi.connecting && kodi.connectedHost
                         && kodi.connectedHost.hwAddr
                enabled: kodi.connectedHost.hwAddr
                onClicked: {
                    kodi.connectedHost.wakeup()
                }
            }
        }
    }
}
