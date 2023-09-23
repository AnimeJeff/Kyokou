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
import "."

Window {
    id: root
    width: 1080
    height: 720
    visible: true
    color: "black"
    flags: Qt.Window | Qt.FramelessWindowHint |Qt.WindowMinimizeButtonHint
    property bool maximised:false
    property bool fullscreen:false
    property bool pipMode: false


    property var mpv : mpvPage.mpv
    property real lastScrollY:0
    property real watchListViewLastY: 0

    property string lastSearch:""
    property bool playerFillWindow:false
    property double lastX
    property double lastY
    property alias resizeAnime: resizingAnimation

    ParallelAnimation {
        id: resizingAnimation
        property real speed:6000
        SmoothedAnimation
        {
            id:widthanime
            target: root
            properties: "width";
            to: Screen.desktopAvailableWidth
            velocity: resizingAnimation.speed
        }
        SmoothedAnimation
        {
            id:heightanime
            target: root
            properties: "height";
            to: Screen.desktopAvailableHeight
            velocity: resizingAnimation.speed
        }
        SmoothedAnimation
        {
            id:xanime
            target: root
            properties: "x";
            velocity: resizingAnimation.speed
            to: 0
        }
        SmoothedAnimation
        {
            id:yanime
            target: root
            properties: "y";
            to: 0
            velocity: resizingAnimation.speed
        }
    }

    Shortcut{
        sequence: "B"
        onActivated:
        {
            //            resizingAnimation.running = true
        }
    }


    onMaximisedChanged:
    {
        if (resizingAnimation.running) return
        if(maximised)
        {
            xanime.to = 0
            yanime.to = 0
            if(root.x !== 0 && root.y !== 0)
            {
                lastX = root.x
                lastY = root.y
            }
            widthanime.to = Screen.desktopAvailableWidth
            heightanime.to = Screen.desktopAvailableHeight
        }
        else
        {
            xanime.to = lastX
            yanime.to = lastY
            widthanime.to = 1080
            heightanime.to = 720
        }
        resizingAnimation.running = true


    }


    onFullscreenChanged:
    {
        if (resizingAnimation.running) return
        if(fullscreen)
        {
            xanime.to = 0
            yanime.to = 0
            if(root.x !== 0 && root.y !== 0)
            {
                lastX = root.x
                lastY = root.y
            }
            widthanime.to = Screen.width
            heightanime.to = Screen.height
        }
        else
        {
            xanime.to = maximised ? 0 : lastX
            yanime.to = maximised ? 0 : lastY
            widthanime.to = maximised ? Screen.desktopAvailableWidth : 1080
            heightanime.to = maximised ? Screen.desktopAvailableHeight : 720
        }
        resizingAnimation.running = true


    }


    onPipModeChanged:
    {
        if (resizingAnimation.running) return
        if (pipMode)
        {
            xanime.to = Screen.desktopAvailableWidth - Screen.width/3
            yanime.to = Screen.desktopAvailableHeight - Screen.height/2.3
            if(root.x !== 0 && root.y !== 0)
            {
                lastX = root.x
                lastY = root.y
            }
            widthanime.to = Screen.width/3
            heightanime.to = Screen.height/2.3
            flags |= Qt.WindowStaysOnTopHint
            sideBar.gotoPage(2)
            playerFillWindow = true
        }
        else
        {
            xanime.to = fullscreen || maximised ? 0 : lastX
            yanime.to = fullscreen || maximised ? 0 : lastY
            widthanime.to = fullscreen ? Screen.width : maximised ? Screen.desktopAvailableWidth : 1080
            heightanime.to = fullscreen ? Screen.height : maximised ? Screen.desktopAvailableHeight : 720
            flags &= ~Qt.WindowStaysOnTopHint
            playerFillWindow = fullscreen
        }
        resizingAnimation.running = true

    }


    Connections
    {
        target: app.playlist
        function onSourceFetched()
        {
            //            mpvPage.mpv.addSubtitle("https://cc.2cdns.com/58/b1/58b108555cd2fc6c93dfeafc08b5e657/58b108555cd2fc6c93dfeafc08b5e657.vtt")
            mpvPage.mpv.subVisible = true
            sideBar.gotoPage(2)
        }
    }

    TitleBar
    {
        id:titleBar
        focus: false
    }

    SideBar
    {
        id:sideBar
        anchors{
            left: parent.left
            top:titleBar.bottom
            bottom:parent.bottom
        }
    }

    StackView
    {
        visible: !mpvPage.visible
        id:stackView
        anchors{
            top: playerFillWindow ? parent.top: titleBar.bottom
            left: playerFillWindow ? parent.left : sideBar.right
            right: parent.right
            bottom: parent.bottom
        }
        initialItem: "explorer/SearchPage.qml"

        background: Rectangle{
            color: "black"
        }
    }

    MpvPage
    {
        id:mpvPage
        visible: false
        anchors.fill: stackView

    }

    Dialog
    {
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
        width: parent.width / 3
        height: parent.height / 4
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

    Timer
    {
        id: timer
        running: false
        repeat: false

        property var callback

        onTriggered: callback()
    }

    function setTimeout(callback, delay)
    {
        if (timer.running)
        {
            console.error("nested calls to setTimeout are not supported!");
            return;
        }
        timer.callback = callback;
        // note: an interval of 0 is directly triggered, so add a little padding
        timer.interval = delay + 1;
        timer.running = true;
    }


    MouseArea
    {
        anchors.fill: parent
        acceptedButtons: Qt.ForwardButton | Qt.BackButton
        onClicked: (mouse)=>
                   {
                       if(playerFillWindow) return;
                       if(mouse.button === Qt.BackButton)
                       {
                           let nextPage = sideBar.currentPage + 1
                           sideBar.gotoPage(nextPage === Object.keys(sideBar.pages).length ? 0 : nextPage)
                       }
                       else
                       {
                           let prevPage = sideBar.currentPage-1
                           sideBar.gotoPage(prevPage < 0 ? Object.keys(sideBar.pages).length-1 : prevPage)
                       }
                   }
    }

    Shortcut
    {
        sequence: "Ctrl+W"
        onActivated:
        {
            if (!pipMode)
            {
                Qt.quit()
            }
        }
    }
    Shortcut
    {
        sequence: "1"
        onActivated: sideBar.gotoPage(0)
    }
    Shortcut
    {
        sequence: "2"
        onActivated: sideBar.gotoPage(1)
    }
    Shortcut
    {
        sequence: "3"
        onActivated: sideBar.gotoPage(2)
    }
    Shortcut
    {
        sequence: "4"
        onActivated: sideBar.gotoPage(3)
    }
    Shortcut
    {
        sequence: "5"
        onActivated: sideBar.gotoPage(4)
    }
    Shortcut
    {
        sequence: "0"
        onActivated: errorPopup.open()
    }
    Shortcut {
        sequence: "Ctrl+Tab"
        onActivated:
        {
            root.showMinimized()
            playerFillWindow = false
            maximised = false
            fullscreen = false
            pipMode = false
            mpv.pause()
            lol.visible = true


        }
    }
    Shortcut {
        sequence: "Ctrl+Q"
        onActivated:
        {
            lol.visible = false
        }
    }
    Rectangle{
        id:lol
        anchors.fill: parent
        visible: false
        color: "black"
    }




    Shortcut
    {
        sequence: "Ctrl+A"
        onActivated:
        {
            pipMode = !pipMode
        }
    }
}
