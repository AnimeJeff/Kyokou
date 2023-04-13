import QtQuick
import QtQuick.Window 2.2
import QtQuick.Controls 2.15

import QtQuick.Layouts 1.15
import MpvPlayer 1.0
import QtQuick.Dialogs
import QtQuick.Controls 2.0

Window {
    id: window
    width: 1080
    height: 720
    visible: true
    color: "black"
    flags: Qt.Window | Qt.FramelessWindowHint |Qt.WindowMinimizeButtonHint
    title: ""//qsTr("kyokou")
    property bool maximised: window.visibility === Window.FullScreen
    property var mpv : mpvPage.mpv

    function setMaximised(shouldFullscreen){
        if(shouldFullscreen){
            window.visibility = Window.FullScreen;
        }else{
            window.visibility = Window.AutomaticVisibility;
        }
    }

    FolderDialog{
        id:folderDialog
        currentFolder: "D:/TV/"
    }
    Connections{
        target: folderDialog
        function onAccepted(){
            app.playlistModel.loadFolder(folderDialog.selectedFolder)
        }

    }


    Connections{
        target: app.searchResultsModel
        function onLoadingStart(){
            loadingScreen.startLoading()

        }
        function onLoadingEnd(){
            loadingScreen.stopLoading()
        }
        function onDetailsLoaded() {
            swipeView.setCurrentIndex(1)
        }
    }

    Connections{
        target: mpv
        function onStateChanged() {
            if(mpv.state === 1){
                mpvPage.visible = true
                contentArea.visible=false
            }
        }
    }

    Connections{
        target: app.playlistModel
        function onSourceFetched(link){
            mpvPage.visible = true
            contentArea.visible=false
            mpv.open(link)
        }
        function onLoadingStart(){
            loadingScreen.startLoading()

        }
        function onLoadingEnd(){
            loadingScreen.stopLoading()
        }
    }

    Rectangle{
        id:viewRect
        anchors.fill: parent
        TitleBar{
            id:titleBar
            focus: false
            function removeMpv(){
                mpvPage.visible=!mpvPage.visible
                contentArea.visible=!mpvPage.visible
            }
            container: window
        }

        Rectangle {
            id: contentArea
            width: parent.width
            height: parent.height - titleBar.height
            color: "black"
            y: titleBar.height

            SwipeView{
                id: swipeView
                anchors.fill: parent
                focus: !mpvPage.visible
                z:0
                currentIndex:0
                SearchPage{
                    id:searchPage
                    Component.onCompleted: {
//                        if(app.playlistModel.onLaunchFile.length === 0){

//                        }
                        app.searchResultsModel.latest(1,3)
                    }
                }

                InfoPage{
                    id:infoPage
                }
                WatchListPage{
                    id:watchListPage
                    swipeView:swipeView
                }
            }
        }

        MpvPage{
            id:mpvPage
            visible: false
            anchors.top: mpvPage.fullscreen ? parent.top:titleBar.bottom

        }



        LoadingScreen{
            id:loadingScreen
            z:10
            anchors{
                top: mpvPage.top
                left:viewRect.left
                right: viewRect.right
                bottom: viewRect.bottom
            }
        }

    }


    property real previousMpvState
    Dialog {
        id: errorPopup
        modal: true
        width: 400
        height: 150
        contentItem: Rectangle {
            color: "#f2f2f2"
            border.color: "#c2c2c2"
            border.width: 1
            radius: 10
            anchors.centerIn: parent
            Text {
                text: "Error"
                font.pointSize: 16
                font.bold: true
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 20
            }
            Text {
                id: errorMessage
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
                onClicked: errorPopup.close()
            }
        }
    }
    Timer {
        id: timer
    }
    function delay(delayTime,cb) {
        timer.interval = delayTime;
        timer.repeat = false;
        timer.triggered.connect(cb);
        timer.start();
    }



}
