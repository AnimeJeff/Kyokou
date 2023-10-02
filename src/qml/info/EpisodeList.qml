import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import "../components"
Item{
    id:episodeList
    Rectangle{
        id:header
        height: 30
        width: episodeList.width
        color: "grey"
        RowLayout {
            anchors.fill: parent
            Text {
                id: countLabel
                Layout.alignment: Qt.AlignTop
                Layout.fillHeight: true
                Layout.fillWidth: true
                text: list.count + " Episodes"
                font.bold: true
                color: "white"
                font.pixelSize: 25
                visible: list.count > 0
            }
            ImageButton {
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                Layout.preferredWidth: height
                Layout.rightMargin: 10
                id:reverseButton
                image: "qrc:/resources/images/sorting-arrows.png"
                hoverImage: "qrc:/resources/images/sorting-arrows.png"
                onClicked: {
                    app.episodeListModel.reversed = !app.episodeListModel.reversed
                }
            }
        }
    }
    ListView {
        id:list
        anchors{
            top: header.bottom
            left: header.left
            right: header.right
            bottom: episodeList.bottom
        }
        ScrollBar.vertical: ScrollBar {
            active: true
        }
        clip: true
        model:app.episodeListModel
        boundsMovement: Flickable.StopAtBounds
        delegate: Rectangle {
            id: delegateRect
            width: list.width
            height: 50 < (episodeStr.height + 10) ? (episodeStr.height + 10) : 50
            color:
            {
                let lastWatchedIndex = showManager.currentShowLastWatchedIndex
                if(lastWatchedIndex === -1)return "black"
                lastWatchedIndex = app.episodeListModel.reversed ? list.count -  lastWatchedIndex - 1 : lastWatchedIndex;
                return lastWatchedIndex === index ? "red":"black"
            }
            Text {
                id:episodeStr
                text:  model.fullTitle
                font.pixelSize: 20 * root.aspectRatio
                anchors{
                    left:parent.left
                    right:parent.right

                    top:parent.top
                    leftMargin: 10
                    rightMargin: 10
                    topMargin: 10
                    bottomMargin: 10
                }
                wrapMode: Text.Wrap
                color: "white"
            }
            MouseArea {
                anchors.fill: delegateRect
                //                onEntered: delegateRect.color = "#ccc"
                //                onExited: delegateRect.color = "#f2f2f2"
                onClicked: (mouse)=>{
                               app.playlist.loadFromEpisodeList(app.episodeListModel.reversed ? list.count - index - 1: index)
                           }
            }

            ImageButton {
                anchors {
                    right: parent.right
                    top: parent.top
                    rightMargin: 10
                    bottom: parent.bottom
                }
                image: "qrc:/resources/images/download-button.png"
                hoverImage: "qrc:/resources/images/download-button.png"
                width: height
                onClicked: {
                    console.log("downloading " + index)
                    app.downloader.downloadCurrentShow(index)
                }
            }



        }
        onCountChanged: {
            if(showManager.currentShowLastWatchedIndex > -1)
            {
                list.positionViewAtIndex(app.episodeListModel.reversed ? list.count - showManager.currentShowLastWatchedIndex - 1 : showManager.currentShowLastWatchedIndex,ListView.Beginning)
            }
        }
    }

}

