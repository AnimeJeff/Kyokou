import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import MpvPlayer 1.0

import "components"

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
        target: showManager
        function onCurrentShowChanged() {
            gotoPage(1)
        }
    }


    property var pages: {
        0: "explorer/SearchPage.qml",
        1: "info/InfoPage.qml",
//        2: mpvPage,
        3: "watchlist/WatchListPage.qml",
        4: "download/DownloadPage.qml",
        5: "settings.qml"
    }

    function gotoPage(index)
    {
        if(fullscreen) return;
        if(index === 1 && !showManager.hasCurrentShow) return;
        if(currentPage!==index)
        {
            currentPage = index
            if(index===2)
            {
                mpvPage.progressBar.peak(2000)
                mpvPage.visible = true
                mpvPage.forceActiveFocus()
            }
            else
            {

                stackView.replace(pages[index])
                stackView.forceActiveFocus()
                setTimeout(()=>{mpvPage.visible = false},100)
            }

        }
    }

    ColumnLayout {
        height: sideBar.height
        spacing: 5
        ImageButton {
            image: selected ? "qrc:/resources/images/search_selected.png" :"qrc:/resources/images/search.png"
            hoverImage: selected ? "qrc:/resources/images/search_selected.png" :"qrc:/resources/images/search.png"
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width
            onClicked: {
                gotoPage(0)
            }
            selected: currentPage === 0
        }

        ImageButton {
            id: detailsPageButton
            enabled: showManager.hasCurrentShow
            image: selected ? "qrc:/resources/images/details_selected.png" : "qrc:/resources/images/details.png"
            hoverImage: selected ? "qrc:/resources/images/details_selected.png" : "qrc:/resources/images/details.png"
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width
            onClicked:gotoPage(1)
            selected: currentPage == 1
        }
        ImageButton {
            id:playerPageButton
            image: selected ? "qrc:/resources/images/tv_selected.png" :"qrc:/resources/images/tv.png"
            hoverImage: selected ? "qrc:/resources/images/tv_selected.png" :"qrc:/resources/images/tv.png"
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width
            onClicked: gotoPage(2)
            selected: currentPage === 2
        }
        ImageButton {
            id:libraryPageButton
            image: selected ? "qrc:/resources/images/library_selected.png" :"qrc:/resources/images/library.png"
            hoverImage: selected ? "qrc:/resources/images/library_selected.png" :"qrc:/resources/images/library.png"
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width

            onClicked: gotoPage(3)
            selected: currentPage === 3
        }
        ImageButton {
            id: downloadPageButton
            image: selected ? "qrc:/resources/images/download_selected.png" :"qrc:/resources/images/download.png"
            hoverImage: selected ? "qrc:/resources/images/download_selected.png" :"qrc:/resources/images/download.png"

            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width
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
            image: selected ? "qrc:/resources/images/settings_selected.png" :"qrc:/resources/images/settings.png"
            hoverImage: selected ? "qrc:/resources/images/settings_selected.png" :"qrc:/resources/images/settings.png"

            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width

            onClicked: gotoPage(5)
            selected: currentPage === 4
        }
    }
}
