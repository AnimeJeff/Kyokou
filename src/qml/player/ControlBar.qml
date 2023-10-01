import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import "../components"
import MpvPlayer 1.0
Control {
    id: controlBar
    required property MpvObject mpv
    background: Rectangle {
        implicitHeight: 40
        color: '#d0303030'
    }
    hoverEnabled: true

    signal playPauseButtonClicked()
    signal settingsButtonClicked()
    signal volumeButtonClicked()
    signal sidebarButtonClicked()
    signal folderButtonClicked()
    signal seekRequested(int time)

    property bool isPlaying: false
    property int time: 0
    property int duration: 0
    property alias volumeButton: volumeButton
    property int buttonSize : 22
    function toHHMMSS(seconds) {
        var hours = Math.floor(seconds / 3600);
        seconds -= hours*3600;
        var minutes = Math.floor(seconds / 60);
        seconds -= minutes*60;

        if (hours   < 10) {hours   = "0"+hours;}
        if (minutes < 10) {minutes = "0"+minutes;}
        if (seconds < 10) {seconds = "0"+seconds;}
        return hours+':'+minutes+':'+seconds;
    }

    RowLayout {
        spacing: 10
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10

        ImageButton {
            id: playPauseButton
            image: isPlaying ? "qrc:/resources/images/pause.png" : "qrc:/resources/images/play.png"
            hoverImage: isPlaying ? "qrc:/resources/images/pause_hover.png" : "qrc:/resources/images/play_hover.png"
            Layout.preferredWidth: buttonSize
            Layout.preferredHeight: buttonSize
            onClicked: playPauseButtonClicked()
        }

        //        ImageButton
        //        {
        //            id: stopButton
        //            image: true ? "qrc:/resources/images/stop_lightgrey.png" : "qrc:/resources/images/stop_grey.png"
        //            hoverImage: true ? "qrc:/resources/images/stop_lightgrey_on.png" : "qrc:/resources/images/stop_grey_on.png"
        //            Layout.preferredWidth: 16
        //            Layout.preferredHeight: 16
        //            onClicked: stopButtonClicked()
        //        }

        ImageButton {
            id: volumeButton
            image: mpv.volume === 0 ? "qrc:/resources/images/mute_volume.png" :
                                      mpv.volume < 25 ? "qrc:/resources/images/low_volume.png" :
                                                        mpv.volume < 75 ? "qrc:/resources/images/mid_volume.png" : "qrc:/resources/images/high_volume.png"

            hoverImage: mpv.volume === 0 ? "qrc:/resources/images/mute_volume_hover.png" :
                                           mpv.volume < 25 ? "qrc:/resources/images/low_volume_hover.png" :
                                                             mpv.volume < 75 ? "qrc:/resources/images/mid_volume_hover.png" : "qrc:/resources/images/high_volume_hover.png"
            Layout.preferredWidth: buttonSize
            Layout.preferredHeight: buttonSize
            onClicked: volumeButtonClicked()
        }

        Text {
            id: timeText
            text: toHHMMSS(time)
            color: "white"
        }

        Slider {
            id: timeSlider
            from: 0
            to: duration
            focusPolicy: Qt.NoFocus
            hoverEnabled: true
            Layout.fillWidth: true
            Layout.preferredHeight: 24
            onPressedChanged: {
                if (!pressed)  // released
                    seekRequested(value);
            }
            background: Rectangle {
                x: timeSlider.leftPadding
                y: timeSlider.topPadding + timeSlider.availableHeight / 2 - height / 2
                implicitWidth: 200
                implicitHeight: 6
                width: timeSlider.availableWidth
                height: implicitHeight
                radius: 2
                color: "#bdbebf"

                Rectangle {
                    width: timeSlider.visualPosition * parent.width
                    height: parent.height
                    color: "#F2F2F2"
                    radius: 2
                }
            }

            handle: Rectangle {
                id:handle
                x: timeSlider.leftPadding + timeSlider.visualPosition * (timeSlider.availableWidth - width)
                y: timeSlider.topPadding + timeSlider.availableHeight / 2 - height / 2
                implicitWidth: timeSlider.hovered ? 20 : 5
                implicitHeight: implicitWidth
                radius: 13
                color: timeSlider.pressed ? "#f0f0f0" : "#f6f6f6"
                border.color: "#bdbebf"
            }
        }

        Text {
            id: durationText
            text: toHHMMSS(duration)
            color: "white"
        }

        ImageButton {
            id: pipButton
            image: "qrc:/resources/images/pip.png"
            hoverImage: "qrc:/resources/images/pip_hover.png"
            Layout.preferredWidth: buttonSize
            Layout.preferredHeight: buttonSize
            onClicked: pipMode = true
        }
        ImageButton {
            id: explorerButton
            image: "qrc:/resources/images/folder.png"
            hoverImage: "qrc:/resources/images/folder_hover.png"
            Layout.preferredWidth: buttonSize
            Layout.preferredHeight: buttonSize
            onClicked: folderButtonClicked()
        }

        ImageButton {
            id: settingsButton
            image: "qrc:/resources/images/player_settings.png"
            hoverImage: "qrc:/resources/images/player_settings_hover.png"
            Layout.preferredWidth: buttonSize
            Layout.preferredHeight: buttonSize
            onClicked: settingsButtonClicked()
        }

        ImageButton {
            id: sidebarButton
            image: "qrc:/resources/images/playlist.png"
            hoverImage: "qrc:/resources/images/playlist_hover.png"
            Layout.preferredWidth: buttonSize
            Layout.preferredHeight: buttonSize
            onClicked: sidebarButtonClicked()
        }
    }

    onTimeChanged:{
        if (!timeSlider.pressed)
            timeSlider.value = time;
    }
}
