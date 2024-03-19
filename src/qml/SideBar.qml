import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material 2.15
import MpvPlayer 1.0
import Qt5Compat.GraphicalEffects
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
        function onCurrentShowChanged(){
            gotoPage(1)
        }
    }


    property var pages: {
        0: "explorer/SearchPage.qml",
        1: "info/InfoPage.qml",
        2: "watchlist/WatchListPage.qml",
        //        3: mpvPage,
        4: "download/DownloadPage.qml",
        5: "settings.qml"
    }

    function gotoPage(index){
        if (fullscreen) return;
        if (index === 1 && !showManager.hasCurrentShow) return;

        if (currentPage!==index){
            currentPage = index
            if (index===3) {
                mpvPage.progressBar.peak(2000)
                mpvPage.visible = true
                mpvPage.forceActiveFocus()
            } else {
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
            source: selected ? "qrc:/resources/images/search_selected.png" :"qrc:/resources/images/search.png"
            // hoverImage: selected ? "qrc:/resources/images/search_selected.png" :"qrc:/resources/images/search.png"
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
            source: selected ? "qrc:/resources/images/details_selected.png" : "qrc:/resources/images/details.png"
            // hoverImage: selected ? "qrc:/resources/images/details_selected.png" : "qrc:/resources/images/details.png"
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width
            onClicked:gotoPage(1)
            selected: currentPage == 1
        }
        ImageButton {
            id:libraryPageButton
            source: selected ? "qrc:/resources/images/library_selected.png" :"qrc:/resources/images/library.png"
            // hoverImage: selected ? "qrc:/resources/images/library_selected.png" :"qrc:/resources/images/library.png"
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width

            onClicked: gotoPage(2)
            selected: currentPage === 2
        }
        ImageButton {
            id:playerPageButton
            source: selected ? "qrc:/resources/images/tv_selected.png" :"qrc:/resources/images/tv.png"
            // hoverImage: selected ? "qrc:/resources/images/tv_selected.png" :"qrc:/resources/images/tv.png"
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width
            onClicked: gotoPage(3)
            selected: currentPage === 3
        }

        ImageButton {
            id: downloadPageButton
            source: selected ? "qrc:/resources/images/download_selected.png" :"qrc:/resources/images/download.png"
            // hoverImage: selected ? "qrc:/resources/images/download_selected.png" :"qrc:/resources/images/download.png"

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





        Item {
            Layout.preferredWidth: sideBar.width
            Layout.preferredHeight: sideBar.width

            Image {
                id: settingsImage
                source: "qrc:/resources/images/settings.png"
                anchors.fill: parent
                visible: true // Hide this and use the layer as the visible entity
                layer.enabled: true
                layer.effect: BrightnessContrast {
                    brightness: 1.0 // Adjust these values as needed
                    contrast: 0.5
                }
            }

            // Use a MouseArea for interaction, if needed
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    onClicked: gotoPage(5)
                    //     // selected: currentPage === 4
                    // Your click handling logic here
                }
            }
        }

        // BrightnessContrast {
        //     anchors.fill: lol
        //     source: lol
        //     brightness: 1.0
        //     contrast: 1.0
        // }

        // Image {
        //     id:lol
        //     // image: selected ? "qrc:/resources/images/settings_selected.png" :"qrc:/resources/images/settings.png"
        //     // hoverImage: selected ? "qrc:/resources/images/settings_selected.png" :"qrc:/resources/images/settings.png"

        //     Layout.preferredWidth: sideBar.width
        //     Layout.preferredHeight: sideBar.width

        //     // onClicked: gotoPage(5)
        //     // selected: currentPage === 4
        //     source: "qrc:/resources/images/settings.png"
        // }
    }
}
