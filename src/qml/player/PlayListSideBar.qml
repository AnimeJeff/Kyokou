import QtQuick
import "./../components"
Rectangle{
    id:playlistBar
    visible: false
    color: '#d0303030'
    CustomComboBox{
        id:playlistComboBox
        anchors{
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: 30
        displayText: app.playlist.showName
        //        model: showManager
        onClickedFun:function(index,model){

        }
    }
    CustomTextField{
        checkedColor: "#727CF5"
        id:playlistSearch
        color: "white"
        anchors{
            top:playlistComboBox.bottom
            left: parent.left
            right: parent.right
        }
        height: 30
        placeholderText: qsTr("Enter query!")
        text: window.lastSearch
        onAccepted: search()
    }

    ListView {
        id: listView
        model: app.playlist
        clip:true
        boundsMovement: Flickable.StopAtBounds
        boundsBehavior: Flickable.StopAtBounds

        cacheBuffer : 100
        anchors{
            top: playlistSearch.bottom
            right: parent.right
            left: parent.left
            bottom: bottomBar.top
        }
        currentIndex: app.playlist.currentIndex

        onCurrentIndexChanged: {
            positionViewAtIndex(currentIndex, ListView.PositionAtCenter)
        }
        spacing : 5
        delegate: Text  {
            id:itemText
            text: model.numberTitle
            color: index === app.playlist.currentIndex ? 'red':"white"
            font.pixelSize: 18
            wrapMode:Text.Wrap
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onDoubleClicked: {
                    app.playlist.play(index)
                }
            }
        }
    }

    Rectangle{
        id:bottomBar
        anchors{
            bottom:parent.bottom
            left: parent.left
            right: parent.right
        }
        color: "#3C4144"
        height: 40
    }
}
