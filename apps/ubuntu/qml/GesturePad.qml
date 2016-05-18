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
import Ubuntu.Components 1.2
import QtFeedback 5.0
import Kodi 1.0

Item {
    id: root

    property int scrollCounter: 0

    Timer {
        id: teaseTimer
        interval: 1000
        running: settings.introStep < Settings.IntroStepDone
        repeat: true
        onTriggered: {
            switch (settings.introStep) {
            case Settings.IntroStepLeftRight:
                leftArrows.item.animate()
                rightArrows.item.animate();
                break;
            case Settings.IntroStepUpDown:
                upArrows.item.animate()
                downArrows.item.animate();
                break;
            case Settings.IntroStepScroll:
                downArrows.item.animate();
                break;
            case Settings.IntroStepColors:
            case Settings.IntroStepClick:
            case Settings.IntroStepExit:
                animateAll();
                break;
            }
        }
    }

    function animateAll() {
        leftArrows.item.animate();
        rightArrows.item.animate();
        upArrows.item.animate();
        downArrows.item.animate();
    }

    HapticsEffect {
        id: rumbleEffect
        intensity: 0.05
        duration: 50
    }

    Component {
        id: arrows
        Row {
            id: arrowsRoot

            function animate() {
                if (!isRunning) {
                    startAnimation();
                } else {
                    onceMore = true;
                }
            }

            signal startAnimation()
            property bool isRunning: false
            property bool onceMore: false

            onIsRunningChanged: {
                if (!isRunning && onceMore) {
                    onceMore = false;
                    startAnimation();
                }
            }

            Repeater {
                id: arrowRepeater
                model: 3

                Icon {
                    id: arrowImage
                    height: units.gu(3)
                    width: height
                    name: "chevron"
                    color: "white"
                    opacity: 0.3

                    Connections {
                        target: arrowsRoot
                        onStartAnimation: {
                            animation.start();
                        }
                    }

                    SequentialAnimation {
                        id: animation

                        onStarted: {
                            if (index === 0 ) {
                                arrowsRoot.isRunning = true;
                            }
                        }

                        onStopped: {
                            if (index === arrowRepeater.count - 1 ) {
                                arrowsRoot.isRunning = false;
                            }
                        }

                        PauseAnimation { duration: 100 * index }
                        NumberAnimation { target: arrowImage; properties: "opacity"; from: 0.3; to: 1; duration: 200 }
                        NumberAnimation { target: arrowImage; properties: "opacity"; from: 1; to: 0.3; duration: 200 }
                    }
                }
            }
        }
    }

    UbuntuShape {
        id: bgImage
        anchors.fill: parent
        color: Qt.rgba(0, 0, 0, 0.05)

        Image {
            anchors.fill: parent
            source: "images/pad-separator.png"
            fillMode: Image.PreserveAspectFit
        }

        Loader {
            id: rightArrows
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: parent.width / 4.5
            sourceComponent: arrows
        }
        Loader {
            id: leftArrows
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: -parent.width / 4.5
            rotation: 180
            sourceComponent: arrows
        }
        Loader {
            id: downArrows
            anchors.centerIn: parent
            anchors.verticalCenterOffset: parent.height / 4.5
            rotation: 90
            sourceComponent: arrows
        }
        Loader {
            id: upArrows
            anchors.centerIn: parent
            anchors.verticalCenterOffset: -parent.height / 4.5
            rotation: -90
            sourceComponent: arrows
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        property int startx
        property int starty
        property int maxClickDistance: 5
        property int minSwipeDistance: 100

        onPressed: {
            startx = mouse.x
            starty = mouse.y
            scrollTimer.start();
        }

        onReleased: {
            scrollTimer.stop();
            if (scrollTimer.triggerCount == 0) {
                doKeyPress(false);
            }
        }

        onPositionChanged: {
            if (scrollTimer.running) {
                var dxAbs = Math.abs(mouseX - startx);
                var dyAbs = Math.abs(mouseY - starty)

                if (dxAbs > dyAbs) {
                    scrollTimer.newSpeed = Math.min(100, Math.max(0, 100 * (dxAbs - minSwipeDistance) / (mouseArea.width - minSwipeDistance)));
                } else {
                    scrollTimer.newSpeed = Math.min(100, Math.max(0, 100 * (dyAbs - minSwipeDistance) / (mouseArea.height - minSwipeDistance)));
                }
            }
        }

        Timer {
            id: scrollTimer
            running: false
            repeat: true
            interval: maxInterval - ((maxInterval - minInterval) * speed / 100);

            property int minInterval: 50
            property int maxInterval: 500
            // Percentage
            property int speed: 0

            // Timer restarts on interval change which blocks it on frequent changes
            // Lets use newSpeed for changing and fetch it when appropriate
            property int newSpeed: -1

            property int triggerCount: 0

            onRunningChanged: {
                if (running) {
                    triggerCount = 0;
                }
            }

            onTriggered: {
                triggerCount++;
                if(newSpeed !== -1) {
                    speed = newSpeed;
                    newSpeed = -1;
                }
                mouseArea.doKeyPress(true);
            }
        }

        function doKeyPress(repeated) {
            var dx = mouseX - startx;
            var dy = mouseY - starty;
            var dxAbs = Math.abs(dx);
            var dyAbs = Math.abs(dy)

            // Did we not move? => press enter
            if (dxAbs < maxClickDistance && dyAbs < maxClickDistance) {
                if (repeated) {
                    // Do not trigger repeated enter presses
                    return;
                }
                if (settings.introStep < Settings.IntroStepDone) {
                    if (settings.introStep == Settings.IntroStepClick || settings.introStep == Settings.IntroStepColors) {
                        settings.introStep++;
                    }
                    // If the user just clicked here during the colors step, let's skip the exit step
                    if (settings.introStep == Settings.IntroStepExit) {
                        settings.introStep++;
                    }

                    return;
                }

                keys.select();
                rumbleEffect.start(1);
                animateAll();
                return;
            }

            // Did we not move more than minSwipeDistance? => do nothing.
            if (dxAbs < minSwipeDistance && dyAbs < minSwipeDistance) {
                print("Only moved " + dx + "x" + dy + " pixels. Not activating gesture");
                return;
            }

            rumbleEffect.start(1);

            if (settings.introStep == Settings.IntroStepScroll && repeated) {
                if (root.scrollCounter < 9) {
                    root.scrollCounter++;
                } else {
                    settings.introStep++;
                }
                return;
            }

            // if horizontal delta is larger than twice the minimum distance,
            // we always go left/right, no matter what the vertical delta is.
            // Reason is that the thumb can easily produce large vertical deltas
            // just by touching the screen with more than the tip
            if (dxAbs > minSwipeDistance * 2 || dxAbs > dyAbs) {
                if (settings.introStep < Settings.IntroStepDone) {
                    if (settings.introStep == Settings.IntroStepLeftRight && !repeated) {
                        settings.introStep++;
                    }
                    return;
                }

                if (dx < 0) {
                    leftArrows.item.animate();
                    keys.left();
                } else {
                    rightArrows.item.animate();
                    keys.right();
                }
            } else {
                if (settings.introStep < Settings.IntroStepDone) {
                    if (settings.introStep == Settings.IntroStepUpDown && !repeated) {
                        settings.introStep++;
                    }
                    return;
                }
                if (dy < 0) {
                    upArrows.item.animate();
                    keys.up();
                } else {
                    downArrows.item.animate();
                    keys.down();
                }
            }
        }
    }
}
