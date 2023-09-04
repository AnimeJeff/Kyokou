import QtQuick 2.15
import QtQuick.Controls 2.15
import MpvPlayer 1.0
import QtQuick.Dialogs
Item{
    id:mpvPage
    property alias mpv:mpvObject
    property alias progressBar:controlBar
    focus: true

    PlayListSideBar{
        id:playlistBar
        anchors{
            right: parent.right
            top: parent.top
            bottom: parent.bottom
        }
        z:2
        width: playlistBar.visible ? 200 : 0
        function toggle(){
            playlistBar.visible = !playlistBar.visible
            if(playlistBar.visible && mpv.state === MpvObject.VIDEO_PLAYING){
                mpv.pause()
            }else if (mpv.state === MpvObject.VIDEO_PAUSED){
                mpv.play()
            }
        }
    }

    Rectangle{
        anchors{
            left:parent.left
            right: playlistBar.left
            top: parent.top
            bottom: parent.bottom
        }
        MpvObject{
            id:mpvObject
            z:0
            volume: volumeSlider.value
            property var lastPos
            anchors.fill: parent
            MouseArea{
                id:mouseArea
                anchors.fill: mpvObject
                hoverEnabled: true
                onMouseXChanged: {
                    mouseArea.cursorShape = Qt.ArrowCursor;
                    controlBar.peak()
                }
                Timer{
                    id:doubleClickTimer
                    interval: 300
                }
                Timer{
                    id:inactivityTimer
                    interval: 2000
                    onTriggered: {
                        if(!mpvPage.visible) return
                        let newPos = cursor.pos()
                        if(newPos === mpvObject.lastPos){
                            cursor.setCursorShape(Qt.BlankCursor)
                        }else{
                            mpvObject.lastPos = cursor.pos()
                            inactivityTimer.restart()
                        }
                    }
                }
                onPositionChanged: {
                    cursor.setCursorShape(Qt.ArrowCursor)
                    mpvObject.lastPos = cursor.pos()
                    inactivityTimer.start()
                }
                acceptedButtons: Qt.LeftButton
                onClicked: (mouse)=>{
                               if(doubleClickTimer.running)
                               {
                                   setPlayerFullscreen(!playerIsFullScreen)
                                   if(mpv.state == MpvObject.VIDEO_PLAYING) {mpv.pause() }else {mpv.play()}
                                   doubleClickTimer.stop()
                               }
                               else{
                                   if(mpv.state == MpvObject.VIDEO_PLAYING) {mpv.pause() }else {mpv.play()}
                                   doubleClickTimer.restart()
                               }


                           }
            }
            ControlBar{
                id:controlBar
                z:1000
                anchors{
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }
                visible: false
                height: 30
                isPlaying: mpv.state === MpvObject.VIDEO_PLAYING || mpv.state === MpvObject.TV_PLAYING
                time: mpv.time
                duration: mpv.duration
                onPlayPauseButtonClicked: mpv.state === MpvObject.VIDEO_PLAYING ? mpv.pause() : mpv.play()
                onStopButtonClicked: mpv.stop()
                onSeekRequested: (time)=>mpv.seek(time);
                onVolumeButtonClicked: {
                    volumePopup.x = mpv.mapFromItem(volumeButton, 0, 0).x;
                    volumePopup.y = mpv.mapFromItem(volumeButton, 0, 0).y - volumePopup.height;
                    volumePopup.visible = true;
                }
                onSidebarButtonClicked: playlistBar.toggle()
                onFolderButtonClicked: folderDialog.open()
                function peak(time){
                    controlBar.visible=true
                    if(time){
                        timer.interval = time
                    }else{
                        timer.interval = 1000
                    }

                    if (autoHideBars) {
                        timer.restart();
                    }
                }
                property bool autoHideBars: true

                Timer {
                    id: timer
                    interval: 1000
                    onTriggered: {
                        if (mouseArea.pressed === true) {
                            return;
                        }
                        if (!controlBar.contains(controlBar.mapFromItem(mouseArea, mouseArea.mouseX, mouseArea.mouseY)))
                        {
                            mouseArea.cursorShape = Qt.BlankCursor;
                            controlBar.visible = false;
                        }
                    }
                }

            }
            Connections{
                target: mpvObject
                function onPlayNext(){
                    app.playlist.playNextItem()
                }
            }
        }

    }

    Popup {
        id: volumePopup
        width: 40
        height: 120
        Slider {
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
    FolderDialog{
        id:folderDialog
        currentFolder: "file:///D:/TV/"
        Connections{
            target: folderDialog
            function onAccepted(){
                app.playlist.loadFolder(folderDialog.selectedFolder)
//todo autoplay
            }
        }
    }


    Keys.enabled: true
    Keys.onPressed: event => handleKeyPress(event)
    function handleCtrlModifiedKeyPress(key){
        switch(key){
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
        }
    }

    function handleKeyPress(event){
        if(event.modifiers & Qt.ControlModifier){
            if(event.key === Qt.Key_W) return
            handleCtrlModifiedKeyPress(event.key)
        }else{
            switch (event.key) {
            case Qt.Key_Escape:
                if (playerIsFullScreen) {
                    setPlayerFullscreen(false);
                }
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
                if (mpv.state === MpvObject.VIDEO_PLAYING) {
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
                // @disable-check M127
                mpv.speed > 1.0 ? mpv.setSpeed(1.0) : mpv.setSpeed(2.0)
                break;
            case Qt.Key_F:
                setPlayerFullscreen(!playerIsFullScreen);
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
                mpv.addSubtitle("https://cc.2cdns.com/58/b1/58b108555cd2fc6c93dfeafc08b5e657/58b108555cd2fc6c93dfeafc08b5e657.vtt")
                break;
            }
        }
    }

    KeyNavigation.tab:mpvPage
}




