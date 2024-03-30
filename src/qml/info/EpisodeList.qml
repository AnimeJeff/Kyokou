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
                source: "qrc:/resources/images/sorting-arrows.png"

                onClicked: {
                    app.currentShow.episodeList.reversed = !app.currentShow.episodeList.reversed
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
        // ScrollBar.vertical: ScrollBar {
        //     active: true
        // }
        clip: true
        model:app.currentShow.episodeList


        Component.onCompleted: {
            list.positionViewAtIndex(app.currentShow.episodeList.lastWatchedIndex, ListView.Center)
        }

        boundsMovement: Flickable.StopAtBounds

        delegate: Rectangle {
            id: delegateRect
            width: list.width
            height: 50 < (episodeStr.height + 10) ? (episodeStr.height + 10) : 50
            border.width: 2
            border.color: "black"
            color: app.currentShow.episodeList.lastWatchedIndex === index ? "red" : "black"
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
                anchors.fill: parent
                hoverEnabled: true
                onEntered: delegateRect.border.color = "white"
                onExited: delegateRect.border.color = delegateRect.color


                onClicked:{
                    app.playFromEpisodeList(index)
                }
            }

            ImageButton {
                source: "qrc:/resources/images/download-button.png"
                height: parent.height * 0.8
                width: parent.height * 0.8
                anchors.right: parent.right


                onClicked: {
                    app.downloadCurrentShow(index)
                    source = "qrc:/resources/images/download_selected.png"
                    enabled = false;
                }
            }


        }

    }

}

