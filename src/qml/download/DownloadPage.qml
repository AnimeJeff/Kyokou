import QtQuick 2.0
import QtQuick.Controls 2.15
import QtQuick.Layouts
Page {
    id: downloadPage
    header: ToolBar {
        contentHeight: 56
        RowLayout {
            anchors.fill: parent
            spacing: 16
            Text {
                text: qsTr("Downloads")
                font.pixelSize: 20
            }
        }
    }

    Component {
        id: progressBarDelegate
        ProgressBar {
            id: progressBar
            width: parent.width
            from: 0
            to: 100
            value: 0
        }
    }


    ListView {
        anchors.fill: parent
        anchors.topMargin: 16
        anchors.bottomMargin: 16
        model: app.downloader
        delegate: ItemDelegate {
            height: 48
            contentItem:
                RowLayout {
                spacing: 16
                Text {
                    text: model.name
                    font.pixelSize: 16
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ProgressBar {
                        from: 0
                        to: 100
                        value: model.progress
                        //                        style: progressBarDelegate
                    }
                }
                Button {
                    text: qsTr("Cancel")
                    font.pixelSize: 16
                    onClicked: {
                        //                        downloadModel.cancelDownload(modelData)
                    }
                }
            }
        }
    }
    focus: true
    //    Rectangle {
    //        id: gameArea
    //        width: 400
    //        height: 400
    //        color: "blue"
    //        focus: true
    //        Image {
    //            id: sprite
    //            source:"qrc:/resources/images/library.png"
    //            width: 50
    //            height: 50
    //            x: gameArea.width/2 - width/2
    //            y: gameArea.height/2 - height/2
    //        }


    //        function moveSprite(x, y) {
    //            sprite.x += x
    //            sprite.y += y
    //        }

    //        Keys.onPressed: {
    //            if (event.key === Qt.Key_Left) {
    //                moveSprite(-1, 0)
    //            } else if (event.key === Qt.Key_Right) {
    //                moveSprite(1, 0)
    //            } else if (event.key === Qt.Key_Up) {
    //                moveSprite(0, -1)
    //            } else if (event.key === Qt.Key_Down) {
    //                moveSprite(0, 1)
    //            }
    //        }
    //    }


}
