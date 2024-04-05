import QtQuick 2.15
import QtQuick.Controls 2.15

Popup {
    id:serverListPopup
    visible: true
    background: Rectangle{
        radius: 10
        color: "black"
    }
    opacity: 0.7
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    modal: true

    Text {
        id: serversText
        text: qsTr("Servers")
        anchors{
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: parent.height / 5
    }
    ListView{
        id:serversListView
        clip: true
        model: app.playlist.serverList
        boundsBehavior: Flickable.StopAtBounds
        anchors {
            top:serversText.bottom
            left: parent.left
            right: parent.right
            bottom:parent.bottom
        }

        delegate: Rectangle {
            width: serversListView.width
            height: 20 * root.fontSizeMultiplier * 3
            color: app.playlist.serverList.currentIndex === index ? "purple" : "black"
            border.width: 2
            border.color: "white"
            Text {
                id:serverText
                text: `${model.name}\n${model.link}`
                font.pixelSize: 20 * root.fontSizeMultiplier
                anchors {
                    left: parent.left
                    right: parent.right
                    top: parent.top
                    bottom: parent.bottom
                    leftMargin: 2
                    rightMargin: 2
                    topMargin: 2
                    bottomMargin: 2
                }

                wrapMode: Text.Wrap
                color: "white"
            }
            MouseArea {
                anchors.fill: parent
                onClicked: app.playlist.serverList.currentIndex = index
            }

        }
    }
}
