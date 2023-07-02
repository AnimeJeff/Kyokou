import QtQuick 2.15

Rectangle{
    id:playlistBar
    visible: false
    color: "white"
    Text {
        id:playlistNameText
        text: app.playlistModel.showName
        font.pixelSize: 16
        font.bold: true
        wrapMode:Text.Wrap
        anchors{
            top: parent.top
            right: parent.right
            left: parent.left
        }
        height: contentHeight
    }
    ListView {
        id: listView
        model: app.playlistModel
        clip:true
        anchors{
            top: playlistNameText.bottom
            right: parent.right
            left: parent.left
            bottom: parent.bottom
        }
        currentIndex: app.playlistModel.currentIndex

        onCurrentIndexChanged: {
            positionViewAtIndex(currentIndex, ListView.PositionAtCenter)
        }
        delegate: Rectangle {
            width: listView.width
            height: itemText.height + 10
            radius: 4
            clip: true
            color: index === app.playlistModel.currentIndex ? 'red': index % 2 === 0 ? "gray" : "white"
            Text  {
                id:itemText
                text: model.numberTitle
                font.pixelSize: 14
                wrapMode:Text.Wrap
                anchors{
                    left: parent.left
                    right: parent.right
                }
            }
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onDoubleClicked: {
                    app.playlistModel.loadSource(index)
                }
            }
        }
    }
}
