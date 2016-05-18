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

import QtQuick 2.4
import QtQuick.Layouts 1.1
import Ubuntu.Components 1.2
import Ubuntu.Components.Popups 1.0
import Ubuntu.Components.ListItems 1.0
import Kodi 1.0
import "components"

KodiPage {
    id: root
    title: model.title
    clip: true

    backVisible: true
    nowPlayingVisible: true
    keypadVisible: true

    onGoToNowPlaying: pageStack.push(nowPlayingPage)
    onGoToKeypad: pageStack.push(keypad)

    property variant model
    property int spacing: units.gu(1)


    Component.onCompleted: {
        console.log("BrowserPage: setting model " + model)
        //filterModel.model = model
        var setting = model.watchedFilterSetting;
        if (setting) {
            settings[setting + 'Changed'].connect(function() {
                filterModel.hideWatched = !settings[setting];
            });
        }
    }

    Component.onDestruction: {
        console.log("model is " + model)
        model.exit();
    }

    // This works around some positioning issue with the SDK header
    Item {
        id: header
        anchors {
            left: parent.left
            top: parent.top
            right: parent.right
        }
        height: 0
    }

    // The delegate for each section header
    Component {
        id: sectionHeading
        Row {
            width: listView.width
            height: 20
            spacing: root.spacing

            Item {
                width: listView.width - sectionHeadingLabel.width - parent.spacing
                height: parent.height
                Rectangle {
                    width: parent.width - root.spacing * 2
                    height: 1
                    color: "#848684"
                    anchors.centerIn: parent
                }
            }

            Label {
                id: sectionHeadingLabel
                text: section
                font.bold: true
                fontSize: "small"
                anchors.verticalCenter: parent.verticalCenter
                color: "#848684"
            }
        }
    }

    KodiFilterModel {
        id: filterModel
        model: root.model
        filterCaseSensitivity: Qt.CaseInsensitive
        filter: searchTextField.text
        hideWatched: model.watchedFilterSetting ? !settings[model.watchedFilterSetting] : false

        onHideWatchedChanged: {
            var setting = model.watchedFilterSetting;
            if (setting) {
                settings[setting] = !hideWatched;
            }
        }
    }

    Item {
        anchors { left: parent.left; top: header.bottom; right: parent.right; bottom: bottomEdge.top }
        clip: true
        z: 2
        TextField {
            id: searchTextField
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: root.spacing
                rightMargin: root.spacing
            }
            y: listView.contentItem.y - height - root.spacing
            opacity: searchBar.expanded ? 1 : 0
            enabled: searchBar.expanded

            primaryItem: Image {
                height: searchTextField.height - units.gu(1)
                width: height
                source: "image://theme/filter"
            }

            Keys.onReturnPressed: {
                listView.forceActiveFocus();
                platformCloseSoftwareInputPanel();
            }

            Behavior on opacity {
                NumberAnimation { duration: 300; easing.type: Easing.OutQuad }
            }

            InverseMouseArea {
                anchors.fill: parent
                enabled: searchTextField.focus
                onPressed: {
                    searchTextField.focus = false;
                    mouse.accepted = false;
                }
            }
        }
    }



    ListView {
        id: listView
        anchors {
            left: parent.left
            top: header.bottom
            right: parent.right
            bottom: bottomEdge.top
        }
        topMargin: searchBar.height
        highlightFollowsCurrentItem: true
        cacheBuffer: itemHeight * 3
        model: filterModel
        clip: true

        property bool draggedForSearch: root.model.allowSearch && contentY < -units.gu(6) && dragging
        property bool useThumbnails: settings.useThumbnails

        property int itemHeight: root.model.thumbnailFormat === KodiModel.ThumbnailFormatPortrait ? units.gu(10) : units.gu(8)

        signal collapse()

        onDraggedForSearchChanged: {
            searchBar.expanded = true;
            print("************************", draggedForSearch, searchBar.height, searchTextField.height)
        }

        Item {
            id: searchBar
            anchors.top: parent.top
            width: parent.width
            height: expanded ? searchTextField.height + root.spacing * 2 : 0
            onHeightChanged: print("fooooooooooooooo", height)
            property bool expanded

            Timer {
                id: searchTimer
                interval: 5000
                repeat: false
                running: searchBar.expanded && !searchTextField.activeFocus && filterModel.filter.length === 0

                onTriggered: {
                    searchBar.expanded = false;
                }
            }


            Behavior on height {
                NumberAnimation { duration: UbuntuAnimation.SnapDuration; easing.type: Easing.OutQuad; }
            }
        }

        delegate:  Item {
            id: delegateItem
            width: parent.width
            height: expanded ? listView.height : collapsedItem.height

            property bool expanded: false

            Behavior on height {
                UbuntuNumberAnimation {}
            }

            function playItem() {
                if (resume == 0) {
                    root.model.playItem(filterModel.mapToSourceIndex(index));
                } else {
                    var popup = PopupUtils.open(Qt.resolvedUrl("components/ResumeDialog.qml"), delegateItem, {item: model });
                    popup.accepted.connect(function() {
                        root.model.playItem(filterModel.mapToSourceIndex(index), true);
                    })
                    popup.rejected.connect(function() {
                        root.model.playItem(filterModel.mapToSourceIndex(index));
                    })
                }
            }

            states: [
                State {
                    name: "expanded"; when: expanded
                    PropertyChanges { target: listView; interactive: false; contentY: index * listView.itemHeight }
                    PropertyChanges { target: fastScroller; enabled: false }
                }
            ]

            Rectangle {
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    bottom: collapsedItem.top
                }

                color: Qt.rgba(0, 0, 0, 0.4)
                opacity: delegateItem.expanded ? 1 : 0
                visible: opacity > 0
                Behavior on opacity {
                    UbuntuNumberAnimation {}
                }

                Loader {
                    id: contentLoader
                    anchors.fill: parent

                    source: delegateItem.expanded ? root.model.getItem(filterModel.mapToSourceIndex(index)).type == "channel" ? "ChannelDetails.qml" : "ItemDetails.qml" : ""

                    onLoaded: {
                        item.item = root.model.getItem(index)
                    }

                    Connections {
                        target: contentLoader.item

                        onPlayItem: {
                                delegateItem.playItem()
                        }

                        onAddToPlaylist: {
                            root.model.addToPlaylist(filterModel.mapToSourceIndex(index))
                        }

                        onDownload: {
                            root.model.download(filterModel.mapToSourceIndex(index), "/home/user/MyDocs/");
                        }
                    }
                }
            }

            ListItemWithActions {
                id: collapsedItem
                height: listView.itemHeight
                width: listView.width
                color: "transparent"
                triggerActionOnMouseRelease: true
                opacity: delegateItem.expanded ? 0.6 : 1

                rightSideActions: [
                    Action {
                        iconName: "media-playback-start"
                        visible: model.playable
                        onTriggered: delegateItem.playItem();
                    },
                    Action {
                        iconName: "add-to-playlist"
                        visible: model.playable
                        onTriggered: root.model.addToPlaylist(filterModel.mapToSourceIndex(index))
                    },
                    Action {
                        iconName: "info"
                        visible: root.model.hasDetails()
                        onTriggered: {
                            root.model.fetchItemDetails(filterModel.mapToSourceIndex(index))
                            delegateItem.expanded = true
                        }
                    }

                ]

                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }

                RowLayout {
                    width: parent.width
                    height: parent.height
                    spacing: units.gu(1)

                    Item {
                        width: root.model.thumbnailFormat === KodiModel.ThumbnailFormatNone ? 0 : thumbnailImage.width
                        visible: root.model.thumbnailFormat !== KodiModel.ThumbnailFormatNone
                        anchors.verticalCenter: parent.verticalCenter
                        height: thumbnailImage.height

                        UbuntuShape {
                            id: thumbnailImage
                            height: listView.itemHeight - units.gu(2)
                            width: root.model.thumbnailFormat === KodiModel.ThumbnailFormatPortrait ?
                                       height * 2/3 :
                                       (root.model.thumbnailFormat === KodiModel.ThumbnailFormatLandscape ?
                                            height * 16/9 :
                                            (root.model.thumbnailFormat === KodiModel.ThumbnailFormat43 ? height * 4/3 : height))
                            anchors.centerIn: parent
                            image: Image {
                                anchors.fill: parent
                                source: thumbnail ? "file://" + thumbnail : ""
                                fillMode: Image.PreserveAspectCrop
                                sourceSize {
                                    width: thumbnailImage.width
                                    height: thumbnailImage.height
                                }
                            }

                            Icon {
                                anchors.centerIn: parent
                                width: Math.min(parent.width, parent.height) - units.gu(1)
                                height: width
                                name: {
                                    switch (root.model.mediaFormat) {
                                    case KodiModel.MediaFormatAudio:
                                        return "audio-x-generic-symbolic";
                                    case KodiModel.MediaFormatVideo:
                                        return "video-x-generic-symbolic";
                                    case KodiModel.MediaFormatPictures:
                                        return "image-x-generic-symbolic";
                                    }
                                    return "empty-symbolic";
                                }
                                color: "white"
                                visible: !thumbnail || thumbnail == "loading"
                            }
                        }
                    }
                    Column {
                        anchors { verticalCenter: parent.verticalCenter }
                        Layout.fillWidth: true
                        height: childrenRect.height
                        spacing: units.gu(.5)

                        Label {
                            width: parent.width
                            text: title
                            elide: Text.ElideRight
                        }
                        Label {
                            width: parent.width
                            text: subtitle + (year.length > 0 ? '\n' + year : "")
                            height: text.length == 0 ? 0 : implicitHeight
                            fontSize: "small"
                            wrapMode: Text.WordWrap
                            maximumLineCount: index >= 0 && root.model.getItem(filterModel.mapToSourceIndex(index)).type === "channel" ? 2 : 3
                        }
                        UbuntuShape {
                            id: progressBar
                            anchors {
                                left:parent.left
                                right: parent.right
                            }
                            visible: index >= 0 ? root.model.getItem(filterModel.mapToSourceIndex(index)).type === "channel" : false
                            color: "#22000000"

                            height: units.dp(3)
                            property int minimumValue: 0
                            property int maximumValue: 100
                            property int value: model.progressPercentage != undefined ? progressPercentage : 0

                            UbuntuShape {
                                anchors.fill: parent
                                anchors.rightMargin: parent.width - (parent.width * parent.value / 100)
                                color: "#1b62c8"
                            }
                        }
                    }
                    Item {
                        Layout.fillHeight: true
                        Layout.preferredWidth: units.gu(2)
                        Icon {
                            width: parent.width
                            height: width
                            color: "white"
                            name: "go-next"
                            visible: filetype == "directory"
                            anchors.verticalCenter: parent.verticalCenter
                        }
                        Icon {
                            anchors {
                                right: parent.right
                                bottom: parent.bottom
                                //bottomMargin: units.gu(1)
                            }
                            width: parent.width
                            height: width
                            visible: playcount > 0 || resume > 0
                            name: resume > 0 ? "media-playback-start" : "tick"
                            color: "white"
                        }
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    enabled: delegateItem.expanded
                    onClicked: delegateItem.expanded = false
                }

                onItemClicked: {
                    if(filetype === "directory") {
                        var component = Qt.createComponent("BrowserPage.qml")
                        if (component.status === Component.Ready) {
                            var newModel = root.model.enterItem(filterModel.mapToSourceIndex(index));
                            newModel.ignoreArticle = settings.ignoreArticle;
                            pageStack.push(component, {model: newModel});
                        } else {
                            console.log("Error loading component:", component.errorString());
                        }
                    } else {
                        delegateItem.playItem();
                    }
                }

                onItemPressAndHold: {
                    if (root.model.hasDetails()) {
                        root.model.fetchItemDetails(filterModel.mapToSourceIndex(index))
                        delegateItem.expanded = true
                    }
                }
            }
        }


        section.property: "sortingTitle"
        section.criteria: ViewSection.FirstCharacter
        // section.delegate: model.parentModel() != null ? sectionHeading : undefined

        Behavior on contentY {
            UbuntuNumberAnimation {}
        }

    }

    MouseArea {
        id:fastScroller
        anchors {top: searchBar.bottom; right: parent.right; bottom: parent.bottom }
        width: units.gu(6)

        Rectangle {
            id: scrollBackground
            color: "black"
            opacity: 0
            anchors.fill: parent

            Behavior on opacity {
                NumberAnimation {
                    easing.type: Easing.InQuad
                    duration: 200
                }
            }
        }

        onReleased: {
            scrollBackground.opacity = 0;
        }

        onCanceled: {
            scrollBackground.opacity = 0;
        }

        onMouseYChanged: {
            scrollBackground.opacity = 0.2;
            var percent = Math.min(model.count - 1, Math.max(0, Math.round((mouseY) / fastScroller.height * (listView.count - 1))))
            scrollIndicatorLabel.text = model.get(percent, "sortingTitle").substring(0, 1);
            listView.positionViewAtIndex(percent, ListView.Center);
            scrollIndicator.y = Math.min(listView.height - scrollIndicator.height + listView.y, Math.max(listView.y, mouseY + fastScroller.y - scrollIndicator.height / 2))
        }

    }
    Item {
        id: scrollIndicator
        opacity: scrollBackground.opacity * 4
        anchors.left: parent.left
        anchors.right: parent.right
        height: units.gu(10)

        Rectangle {
            anchors.fill: parent
            color: "black"
        }
        Label {
            id: scrollIndicatorLabel
            anchors.fill: scrollIndicator
            verticalAlignment: Text.AlignVCenter
            anchors.margins: units.gu(2)
            color: "white"
            fontSize: "x-large"
        }
    }

    ActivityIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: model.busy
        visible: model.busy
    }

    BottomEdge {
        id: bottomEdge

        BottomEdgeButton {
            text: qsTr("Home")
            source: "image://theme/go-home"
            Layout.fillWidth: true
            onClicked: {
                print("pageStack depth:", pageStack.depth)
                pageStack.clear();
                pageStack.push(mainPage)
            }
        }
        BottomEdgeButton {
            text: filterModel.hideWatched ? qsTr("Show watched") : qsTr("Hide watched")
            source: filterModel.hideWatched ? "../images/unchecked.svg" : "image://theme/select"
            visible: model.allowWatchedFilter
            Layout.fillWidth: true
            onClicked: {
                filterModel.hideWatched = !filterModel.hideWatched
            }
        }
        BottomEdgeButton {
            text: qsTr("Sorting")
            source: filterModel.sortOrder == Qt.AscendingOrder ? "image://theme/go-down" : "image://theme/go-up"
            Layout.fillWidth: true
            onClicked: {
                filterModel.sortOrder = filterModel.sortOrder == Qt.AscendingOrder ? Qt.DescendingOrder : Qt.AscendingOrder
            }
        }
    }
}
