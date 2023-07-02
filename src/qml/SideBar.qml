import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import MpvPlayer 1.0

import "components"


// Sidebar UI
Rectangle {

    id: sideBar

    width: 50
    height: parent.height

    color: "black"
    //    color: "white"
    // Pane for elevation shadow
    Pane {
        anchors.fill: parent
        Material.background: parent.color
        Material.elevation: 1
    }
    property int currentPage: 0
    // Positionate all buttons
    Connections{
        target: app.showExplorer
        function onDetailsLoaded() {
            gotoPage(1)
        }
    }

    Connections{
        target: app.playlistModel
        function onSourceFetched(link){
            mpv.open(link)
            gotoPage(2)
        }
    }
    property var pages: {
        0: "explorer/SearchPage.qml", //todo keep track of location of last scroll and scroll to
        1: "info/InfoPage.qml",
        2: mpvPage,
        3: "watchlist/WatchListPage.qml",
        4: "download/DownloadPage.qml"
    }

    function gotoPage(index){
        if(currentPage!==index){
            if(index===2){
                mpvPage.progressBar.peak(2000)
                mpvPage.visible = true
                mpvPage.forceActiveFocus()
            }else{
                stackView.replace(pages[index])
                stackView.forceActiveFocus()
                timer.start();
            }
            currentPage = index
        }
    }
    Timer{
        id:timer
        repeat: false
        interval: 100
        onTriggered: {
            mpvPage.visible = false
        }
    }
    ColumnLayout {
        height: sideBar.height
        spacing: 0
        ImageButton {
            image:"qrc:/resources/images/search.png"
            hoverImage:"qrc:/resources/images/search.png"
            HoverCursorArea{}
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width
            onClicked: {
                gotoPage(0)
            }
            selected: currentPage === 0
        }

        ImageButton {
            image:"qrc:/resources/images/view-details.png"
            hoverImage:"qrc:/resources/images/view-details.png"
            HoverCursorArea{}
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width
            onClicked: {
                gotoPage(1)
            }
            selected: currentPage === 1
        }
        ImageButton {
            image:"qrc:/resources/images/retro-tv.png"
            hoverImage:"qrc:/resources/images/retro-tv.png"
            HoverCursorArea{}
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width
            onClicked: gotoPage(2)
            selected: currentPage === 2
        }
        ImageButton {
            image:"qrc:/resources/images/library.png"
            hoverImage:"qrc:/resources/images/library.png"
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width
            HoverCursorArea{}
            onClicked: gotoPage(3)
            selected: currentPage === 3
        }

        ImageButton {
            image:"qrc:/resources/images/download.png"
            hoverImage:"qrc:/resources/images/download.png"
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width
            HoverCursorArea{}
            onClicked: gotoPage(4)
            selected: currentPage === 4
        }
        AnimatedImage {
            source: "qrc:/resources/gifs/basketball.gif"
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width
            MouseArea {cursorShape: Qt.PointingHandCursor}
        }
        Rectangle {
            property string orientation: "vertical"
            color: "transparent"

            Layout.fillWidth: orientation == "horizontal"
            Layout.fillHeight: orientation == "vertical"
        }
        ImageButton {
            image:"qrc:/resources/images/settings.png"
            hoverImage:"qrc:/resources/images/settings.png"
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width
            HoverCursorArea{}
        }
    }
}
