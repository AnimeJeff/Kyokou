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
            height: 60
            border.width: 2
            border.color: "black"
            color: app.currentShow.episodeList.lastWatchedIndex === index ? "red" : "black"
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: delegateRect.border.color = "white"
                onExited: delegateRect.border.color = delegateRect.color
                onClicked:{
                    console.log(index)
                    app.playFromEpisodeList(index)
                }
            }
            RowLayout {
                anchors{
                    left:parent.left
                    right:parent.right
                    top:parent.top
                    bottom: parent.bottom
                    margins: 3
                }

                Text {
                    id:episodeStr
                    text:  model.fullTitle
                    font.pixelSize: 20 * root.fontSizeMultiplier
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    elide: Text.ElideRight
                    wrapMode: Text.Wrap
                    color: "white"
                }

                ImageButton {
                    source: "qrc:/resources/images/download-button.png"
                    Layout.preferredWidth: height
                    Layout.preferredHeight: 50
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                    onClicked: {
                        enabled = false;
                        source = "qrc:/resources/images/download_selected.png"
                        app.downloadCurrentShow(index)
                    }
                }

            }



        }

    }

}

