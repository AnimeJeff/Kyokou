import QtQuick
import QtQuick.Window 2.2
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import MpvPlayer 1.0

import "./explorer"
import "./info"
import "./player"
import "./watchlist"
import "./download"
import "./components"

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
    property real lastScrollY:0
    property string lastSearch:""
    property bool playerIsFullScreen:false

    function setMaximised(fullscreen){
        if(fullscreen){
            window.visibility = Window.FullScreen;
        }else{
            window.visibility = Window.AutomaticVisibility;
        }
    }

    function setPlayerFullscreen(fullscreen){
        setMaximised(fullscreen)
        playerIsFullScreen = fullscreen
    }

    Connections{
        target: app.playlist
        function onSourceFetched(link){
            mpvPage.mpv.open(link)
            mpvPage.mpv.addSubtitle("https://cc.2cdns.com/58/b1/58b108555cd2fc6c93dfeafc08b5e657/58b108555cd2fc6c93dfeafc08b5e657.vtt")
            mpvPage.mpv.subVisible = true
            sideBar.gotoPage(2)
        }
    }

    Rectangle{
        id:viewRect
        anchors.fill: parent
        TitleBar{
            id:titleBar
            focus: false
        }
        SideBar{
            id:sideBar
            anchors{
                left: parent.left
                top:titleBar.bottom
                bottom:parent.bottom
            }
        }

        StackView{
            visible: !mpvPage.visible
            id:stackView
            anchors{
                top: playerIsFullScreen ? parent.top: titleBar.bottom
                left: playerIsFullScreen ? parent.left : sideBar.right
                right: parent.right
                bottom: parent.bottom
            }
            initialItem: "explorer/SearchPage.qml"

            background: Rectangle{
                color: "black"
            }
        }

        MpvPage{
            id:mpvPage
            visible: false
            anchors.fill: stackView

        }


        Dialog {
            Connections{
                target: errorHandler
                function onShowWarning(msg){
                    errorMessage.text = msg
                    errorPopup.open()
                }
            }

            anchors.centerIn: parent
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

    }

    MouseArea{
        anchors.fill: parent
        acceptedButtons: Qt.ForwardButton | Qt.BackButton
        onClicked: (mouse)=>{
                       if(playerIsFullScreen)return;
                       if(mouse.button === Qt.BackButton){
                           let nextPage = sideBar.currentPage+1
                           sideBar.gotoPage(nextPage === Object.keys(sideBar.pages).length ? 0 : nextPage)
                       }else{
                           let prevPage = sideBar.currentPage-1
                           sideBar.gotoPage(prevPage < 0 ? Object.keys(sideBar.pages).length-1 : prevPage)
                       }
                   }
    }

    Component.onCompleted: {
        if(app.playlist.onLaunchFile){
            timer.start()
        }
    }
    Timer {
            id: timer
            interval: 1000
            running: false
            repeat: false
            onTriggered: {
                mpv.open(app.playlist.onLaunchFile)
                sideBar.gotoPage(2)
            }
        }





    Shortcut {
        sequence: "Ctrl+W"
        onActivated: Qt.quit()
    }
    Shortcut {
        sequence: "1"
        onActivated: sideBar.gotoPage(0)
    }
    Shortcut {
        sequence: "2"
        onActivated: sideBar.gotoPage(1)
    }
    Shortcut {
        sequence: "3"
        onActivated: sideBar.gotoPage(2)
    }
    Shortcut {
        sequence: "4"
        onActivated: sideBar.gotoPage(3)
    }
    Shortcut {
        sequence: "5"
        onActivated: sideBar.gotoPage(4)
    }
    Shortcut {
        sequence: "0"
        onActivated: errorPopup.open()
    }
}
