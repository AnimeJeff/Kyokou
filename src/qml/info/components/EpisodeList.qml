import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
Item{
    id:episodeList
    Rectangle{
        id:header
        height: 30
        width: episodeList.width
        color: "grey"
        RowLayout{
            anchors.fill: parent
            Label{
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
            Button{
                Layout.fillHeight: true

                Layout.alignment: Qt.AlignRight
                Layout.preferredWidth: 100
                id:reverseButton
                text: "Reverse"

                font.pixelSize: 25
                onClicked: {
                    app.episodeListModel.reversed = !app.episodeListModel.reversed
                }
                visible: true
                background: Rectangle{
                    color: "white"
                }

            }
        }
    }
    ListView{
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
            color: {
                let lastWatchedIndex = showManager.currentShowLastWatchedIndex
                if(lastWatchedIndex === -1)return "black"
                lastWatchedIndex = app.episodeListModel.reversed ? list.count -  lastWatchedIndex - 1 : lastWatchedIndex;
                return lastWatchedIndex === index ? "red":"black"
            }
            Text {
                id:episodeStr
                text:  "Episode " + model.number.toString() + (model.title === undefined || parseInt(model.title) === model.number ? "" : "\n" + model.title)
                font.pixelSize: 16

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
                font.family: "Arial"
            }
            MouseArea {
                anchors.fill: delegateRect
                //                onEntered: delegateRect.color = "#ccc"
                //                onExited: delegateRect.color = "#f2f2f2"
                onClicked: (mouse)=>{
                               app.loadSourceFromList(app.episodeListModel.reversed ? list.count - index - 1 : index)
                           }
            }

        }
        onCountChanged: {
            if(showManager.currentShowLastWatchedIndex>-1){
                list.positionViewAtIndex(showManager.currentShowLastWatchedIndex,ListView.Beginning)
            }
        }
    }
}

