import QtQuick 2.15
import QtQuick.Controls 2.15
import MpvPlayer 1.0
import QtQuick.Dialogs
import QtQuick.Layouts 1.15
Item{
    id:mpvPage
    property alias progressBar:controlBar
    focus: true
    visible: false

    MpvObject {
        id:mpvObject
        z:0
        volume: volumeSlider.value
        property var lastPos
        anchors {
            left:parent.left
            right: playlistBar.left
            top: parent.top
            bottom: parent.bottom
        }
        onPlayNext: app.playlist.playNextItem()
        Component.onCompleted: {
            root.mpv = mpvObject
            if (app.playlist.launchPath.toString().trim() !== "") {
                sideBar.gotoPage(3)
                setTimeout(()=>mpv.open(app.playlist.launchPath), 100)
            } else {
                app.showExplorer.latest(1,4);
            }
        }
        Rectangle {
            id:pipCloseButton
            width: 20
            height: 20
            z: mouseArea.z + 1
            anchors {
                top: mpvObject.top
                left:mpvObject.left
            }
            visible: pipMode && mouseArea.containsMouse
            MouseArea {
                preventStealing: true
                anchors.fill: parent
                onClicked: {
                    //                    pipCloseButton.visible = false
                    pipMode = false
                }
            }
            color:"red"
        }

        MouseArea {
            id: mouseArea
            property var clickPos
            property point lastMousePosition
            hoverEnabled: true
            acceptedButtons: Qt.LeftButton

            anchors {
                top: mpvObject.top
                bottom: controlBar.visible ? controlBar.top : mpvObject.bottom
                left: mpvObject.left
                right: mpvObject.right
            }

            onPressed: (mouse) => {
                           if (pipMode) {
                               // If in PiP mode, store the initial click position for dragging
                               clickPos = Qt.point(mouse.x, mouse.y);
                           }
                       }

            onReleased: (mouse) => {
                            if (pipMode) {
                                clickPos = null; // Reset click position after drag
                                if (doubleClickTimer.running) {
                                    pipMode = false;
                                } else {
                                    doubleClickTimer.restart();
                                }
                            } else {
                                if (mpv.state == MpvObject.VIDEO_PLAYING) mpv.pause(); else mpv.play();

                                if (doubleClickTimer.running) {
                                    root.playerFillWindow = !root.playerFillWindow
                                    root.fullscreen = root.playerFillWindow
                                } else {
                                    doubleClickTimer.restart();
                                }
                            }
                        }

            onPositionChanged: (mouse) => {
                                   if (pipMode && clickPos) {
                                       // Handle drag in PiP mode
                                       let newX = mouse.x - clickPos.x + root.x;
                                       let newY = mouse.y - clickPos.y + root.y;
                                       root.x = Math.max(0, Math.min(Screen.desktopAvailableWidth - root.width, newX));
                                       root.y = Math.max(0, Math.min(Screen.desktopAvailableHeight - root.height, newY));
                                   } else {
                                       // Handle cursor showing and auto-hide logic
                                       controlBar.peak();
                                       app.cursor.visible = true;
                                       lastMousePosition = Qt.point(mouse.x, mouse.y);
                                       inactivityTimer.restart();
                                   }
                               }

            onEntered: {
                mouseArea.cursorShape = Qt.ArrowCursor
            }

            Timer {
                id: doubleClickTimer
                interval: 300 // Double-click threshold
            }

            Timer {
                id: inactivityTimer
                interval: 2000
                onTriggered: {
                    if (!mpvPage.visible) return
                    var newPos = Qt.point(mouseArea.mouseX, mouseArea.mouseY)
                    if (newPos === mouseArea.lastMousePosition && !controlBar.hovered &&
                            !mouseArea.pressed && !settingsPopup.visible && !serverListPopup.visible
                            && mouseArea.containsMouse
                            ) {
                        // If the mouse hasn't moved, hide the cursor
                        app.cursor.visible = false;
                        controlBar.visible = false;
                    }
                    else {
                        // If the mouse has moved, update the last position and restart the timer
                        mouseArea.lastMousePosition = newPos;
                        inactivityTimer.restart();
                    }
                }
            }
        }

        ControlBar {
            id:controlBar
            z: mpvObject.z + 1
            property bool autoHideBars: true
            anchors {
                bottom: parent.bottom
                left: parent.left
                right: parent.right
            }
            visible: false
            height: parent.height/12.5
            isPlaying: mpv.state == MpvObject.VIDEO_PLAYING || mpv.state === MpvObject.TV_PLAYING
            time: mpv.time
            duration: mpv.duration
            mpv:mpvObject
            function peak(time){
                if (pipMode) return
                controlBar.visible = true
                if (time) {
                    inactivityTimer.interval = time
                }
                else {
                    inactivityTimer.interval = 1000
                }

                if (autoHideBars) {
                    inactivityTimer.restart();
                }
            }

            onPlayPauseButtonClicked: mpv.state === MpvObject.VIDEO_PLAYING ? mpv.pause() : mpv.play()
            onSeekRequested: (time)=>mpv.seek(time);
            onSidebarButtonClicked: playlistBar.toggle()
            onFolderButtonClicked: folderDialog.open()
            onSettingsButtonClicked: settingsPopup.opened ? settingsPopup.close() : settingsPopup.open()
            onServersButtonClicked: serverListPopup.opened ? serverListPopup.close() : serverListPopup.open()
            onVolumeButtonClicked: {
                volumePopup.x = mpv.mapFromItem(volumeButton, 0, 0).x;
                volumePopup.y = mpv.mapFromItem(volumeButton, 0, 0).y - volumePopup.height;
                volumePopup.visible = true;
            }

            Popup {
                id: volumePopup
                width: 40
                height: 120
                Slider
                {
                    id: volumeSlider
                    from: 0
                    to: 100
                    value: 50
                    stepSize: 1
                    snapMode: Slider.SnapAlways
                    anchors.fill: parent
                    orientation: Qt.Vertical
                }
            }

        }

        SettingsPopup {
            id:settingsPopup
            x:parent.width-width - 10
            y:parent.height - height - controlBar.height - 10
            width: parent.width / 3.5
            height: parent.height / 3.5
        }

        ServerListPopup {
            id:serverListPopup
            anchors.centerIn: parent
            visible: false
            width: parent.width / 3
            height: parent.height / 2.5

        }
    }

    Dialog {
        id: notifier
        modal: true
        width: parent.width / 3
        height: parent.height / 4
        anchors.centerIn: parent
        focus: false
        property alias headerText:headerText.text
        property alias text:notifierMessage.text

        contentItem: Rectangle {
            color: "#f2f2f2"
            border.color: "#c2c2c2"
            border.width: 1
            radius: 10
            anchors.centerIn: parent
            Text {
                id:headerText
                text: "Error"
                font.pointSize: 16
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 20
            }
            Text {
                id: notifierMessage
                text: "An error has occurred."
                font.pointSize: 14
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
            }
            Button {
                text: "OK"
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 10
                anchors.horizontalCenter: parent.horizontalCenter
                onClicked: notifier.close()
            }
        }
        Connections {
            target: errorHandler
            function onShowWarning(msg){
                notifierMessage.text = msg
                notifier.open()
            }
        }

    }

    PlayListSideBar {
        id:playlistBar
        anchors{
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        z:2
        width: playlistBar.visible ? root.width/5 : 0

        function toggle()
        {
            playlistBar.visible = !playlistBar.visible
            if (playlistBar.visible && mpv.state === MpvObject.VIDEO_PLAYING)
            {
                mpv.pause()
            }
            else if (mpv.state === MpvObject.VIDEO_PAUSED)
            {
                mpv.play()
            }
        }
    }

    FolderDialog {
        id:folderDialog
        currentFolder: "file:///D:/TV/"
        onAccepted:
        {
            app.playlist.replaceCurrentPlaylist(folderDialog.selectedFolder)
        }
    }



    Keys.enabled: true
    Keys.onPressed: event => handleKeyPress(event)
    Keys.onReleased: event => {
                         switch(event.key) {
                             case Qt.Key_Shift:
                             mpv.setSpeed(mpv.speed / 2)
                         }
                     }


    function handleCtrlModifiedKeyPress(key){
        switch(key)
        {
        case Qt.Key_1:
            mpv.loadAnime4K(1)
            break;
        case Qt.Key_2:
            mpv.loadAnime4K(2)
            break;
        case Qt.Key_3:
            mpv.loadAnime4K(3)
            break;
        case Qt.Key_4:
            mpv.loadAnime4K(4)
            break;
        case Qt.Key_5:
            mpv.loadAnime4K(5)
            break;
        case Qt.Key_0:
            mpv.loadAnime4K(0)
            break;
        case Qt.Key_Z:
            mpv.seek(mpv.time - 90)
            break;
        case Qt.Key_X:
            mpv.seek(mpv.time + 90)
            break;
        case Qt.Key_S:
            app.playlist.playPrecedingItem()
            break;
        case Qt.Key_D:
            app.playlist.playNextItem()
            break;
        case Qt.Key_V:
            mpv.pasteOpen()
            break;
        case Qt.Key_R:
            mpv.reload()
            break;
        case Qt.Key_C:
            mpv.copyVideoLink()
            break;
        }
    }

    function handleKeyPress(event){
        if (event.modifiers & Qt.ControlModifier){
            if (event.key === Qt.Key_W) return
            handleCtrlModifiedKeyPress(event.key)
        }else{
            switch (event.key){
            case Qt.Key_Escape:
                if (resizeAnime.running) return
                if (pipMode)
                {
                    pipMode = false
                    return
                }
                playerFillWindow = false
                fullscreen = false
                break;
            case Qt.Key_P:
                playlistBar.toggle();
                break;
            case Qt.Key_W:
                playlistBar.visible = !playlistBar.visible;
                break;
            case Qt.Key_Up:
                volumeSlider.value += 5;
                break;
            case Qt.Key_Down:
                volumeSlider.value -= 5;
                break;
            case Qt.Key_Q:
                volumeSlider.value += 5;
                break;
            case Qt.Key_A:
                volumeSlider.value -= 5;
                break;
            case Qt.Key_Space:
            case Qt.Key_Clear:
                if (mpv.state === MpvObject.VIDEO_PLAYING){
                    mpv.pause();
                } else {
                    mpv.play();
                }
                break;
            case Qt.Key_PageUp:
                app.playlist.playNextItem();
                break;
            case Qt.Key_Home:
                app.playlist.playPrecedingItem();
                break;
            case Qt.Key_PageDown:
                mpv.seek(mpv.time + 90);
                break;
            case Qt.Key_End:
                mpv.seek(mpv.time - 90);
                break;
            case Qt.Key_Plus:
            case Qt.Key_D:
                mpv.setSpeed(mpv.speed + 0.1);
                break;
            case Qt.Key_Minus:
            case Qt.Key_S:
                mpv.setSpeed(mpv.speed - 0.1);
                break;
            case Qt.Key_R:
                if (mpv.speed > 1.0)
                    mpv.setSpeed(1.0)
                else
                    mpv.setSpeed(2.0)
                break;
            case Qt.Key_F:
                if (resizeAnime.running) return
                if (pipMode)
                {
                    pipMode = false
                    return
                }
                playerFillWindow = !playerFillWindow
                fullscreen = playerFillWindow
                break;
            case Qt.Key_M:
                mpv.mute();
                break;
            case Qt.Key_Z:
            case Qt.Key_Left:
                mpv.seek(mpv.time - 5);
                break;
            case Qt.Key_X:
            case Qt.Key_Right:
                mpv.seek(mpv.time + 5);
                break;
            case Qt.Key_Tab:
            case Qt.Key_Asterisk:
                mpv.showText(app.playlist.currentItemName);
                break;
            case Qt.Key_Slash:
                controlBar.peak()
                break;
            case Qt.Key_C:
                break;
            case Qt.Key_Shift:
                mpv.setSpeed(mpv.speed * 2)
            }
        }
    }

    KeyNavigation.tab:mpvPage
}



// MouseArea {
//     id:mouseArea
//     anchors {
//         top: mpvObject.top
//         bottom: controlBar.visible ? controlBar.top : mpvObject.bottom
//         left: mpvObject.left
//         right: mpvObject.right
//     }
//     hoverEnabled: true
//     onExited: {
//         app.cursor.visible = true
//     }
//     onPositionChanged: {
//         if (!pipMode)
//         {
//             controlBar.peak()
//             app.cursor.visible = true
//             mpvObject.lastPos = app.cursor.pos()
//             inactivityTimer.start()
//         }
//     }
//     acceptedButtons: Qt.LeftButton
//     onClicked: (mouse)=> {
//                    if (doubleClickTimer.running)
//                    {
//                        if (pipMode)
//                        {
//                            pipMode = false
//                            return
//                        }
//                        root.playerFillWindow = !root.playerFillWindow
//                        root.fullscreen = root.playerFillWindow
//                        if (mpv.state == MpvObject.VIDEO_PLAYING)
//                        {
//                            mpv.pause()
//                        }
//                        else
//                        {
//                            mpv.play()
//                        }
//                        doubleClickTimer.stop()
//                    }
//                    else
//                    {
//                        if (mpv.state == MpvObject.VIDEO_PLAYING) mpv.pause()
//                        else mpv.play()

//                        doubleClickTimer.restart()
//                    }
//                }


//     MouseArea {
//         id:pipModeMouseArea
//         pressAndHoldInterval:300
//         visible: pipMode
//         enabled: pipMode
//         hoverEnabled: true
//         anchors.fill: parent
//         property var clickPos
//         onPositionChanged: {
//             if (clickPos)
//             {
//                 let newX = app.cursor.pos().x - clickPos.x
//                 let newY = app.cursor.pos().y - clickPos.y
//                 if (newX < 0) newX = 0
//                 else if (newX > Screen.desktopAvailableWidth-root.width) newX = Screen.desktopAvailableWidth-root.width
//                 if (newY < 0) newY = 0
//                 else if (newY > Screen.desktopAvailableHeight-root.height) newY = Screen.desktopAvailableHeight-root.height
//                 root.x = newX
//                 root.y = newY
//             }
//         }
//         onReleased: {
//             clickPos = null
//         }
//         onPressed: (mouse)=>{
//                        clickPos  = Qt.point(mouse.x,mouse.y)
//                    }
//         onContainsMouseChanged: {
//             pipCloseButton.visible = containsMouse
//         }
//     }


//     Timer {
//         id:doubleClickTimer
//         interval: 300
//     }
//     Timer {
//         id:inactivityTimer
//         interval: 2000
//         onTriggered:
//         {
//             if (!mpvPage.visible) return
//             let newPos = app.cursor.pos()
//             if (newPos === mpvObject.lastPos && !controlBar.hovered)
//             {
//                 app.cursor.visible = false
//             }
//             else
//             {
//                 mpvObject.lastPos = app.cursor.pos()
//                 inactivityTimer.restart()
//             }
//         }
//     }

// }

// Timer {
//     id: timer
//     interval: 1000
//     onTriggered:
//     {
//         if (!mouseArea.pressed &&
//                 !controlBar.contains(controlBar.mapFromItem(mouseArea, mouseArea.mouseX, mouseArea.mouseY)) &&
//                 !settingsPopup.visible
//                 )
//         {
//             mouseArea.cursorShape = Qt.BlankCursor;
//             controlBar.visible = false;
//         }else{
//             timer.restart()
//         }
//     }
// }
