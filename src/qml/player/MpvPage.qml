import QtQuick 2.15
import QtQuick.Controls
import MpvPlayer 1.0
import QtQuick.Dialogs
import QtQuick.Layouts 1.15
Item{
    id:mpvPage

    property alias playListSideBar: playlistBar
    Connections {
        target: app.playList
        function onSourceFetched() {
            mpv.subVisible = true
            sideBar.gotoPage(3)
        }
    }

    MpvPlayer {
        id:mpvPlayer
        focus: true
        anchors {
            left:parent.left
            right: playlistBar.left
            top: parent.top
            bottom: parent.bottom
        }
    }

    onVisibleChanged: if(visible) mpvPlayer.forceActiveFocus()
    PlayListSideBar {
        id:playlistBar
        anchors{
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        width: playlistBar.visible ? root.width/5 : 0
        visible: false
        function toggle() {
            if (root.pipMode) return
            playlistBar.visible = !playlistBar.visible
            if (playlistBar.visible && mpv.state === MpvObject.VIDEO_PLAYING) {
                mpv.pause()
            } else if (mpv.state === MpvObject.VIDEO_PAUSED) {
                mpv.play()
            }
        }
    }



    KeyNavigation.tab:mpvPlayer
}

